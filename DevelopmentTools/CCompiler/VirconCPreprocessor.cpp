// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "VirconCPreprocessor.hpp"
    #include "VirconCLexer.hpp"
    #include "CompilerInfrastructure.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// maximum number of times to repeat macro expansion in a
// same line, in case definitions use other definitions
#define MAX_EXPANSION_PASSES 10


// =============================================================================
//      PROCESSING CONTEXT CLASS
// =============================================================================


ProcessingContext::ProcessingContext()
{
    LinePosition = SourceLines.begin();
}

// -----------------------------------------------------------------------------

ProcessingContext::~ProcessingContext()
{
    // delete all cloned tokens
    for( CTokenList Line: SourceLines )
      for( CToken* T: Line )
        delete T;
}

// -----------------------------------------------------------------------------

void ProcessingContext::Advance()
{
    if( LinePosition != SourceLines.end() )
      LinePosition++;
}

// -----------------------------------------------------------------------------

bool ProcessingContext::LinesHaveEnded()
{
    return (SourceLines.empty() || (LinePosition == SourceLines.end()));
}

// -----------------------------------------------------------------------------

CTokenList& ProcessingContext::GetCurrentLine()
{
    if( LinesHaveEnded() )
      throw runtime_error( "preprocessing context tried to access more lines than available" );
    
    return *LinePosition;
}

// -----------------------------------------------------------------------------

bool ProcessingContext::AreAllIfConditionsMet()
{
    // check if some #if condition is
    // not met at the current line
    for( auto Context: IfStack )
    {
        bool ConditionMet = (Context.ElseWasFound? !Context.ConditionIsMet : Context.ConditionIsMet);
        
        if( !ConditionMet )
          return false;
    }
    
    // no condition failed
    // (or there was none)
    return true;
}


// =============================================================================
//      VIRCON C PREPROCESSOR: INSTANCE HANDLING
// =============================================================================


VirconCPreprocessor::~VirconCPreprocessor()
{
    // delete all processed tokens
    for( CToken* T: ProcessedTokens )
      delete T;
    
    // delete all function macro body tokens
    for( auto& Pair: FunctionDefinitions )
      for( CToken* T: Pair.second.Body )
        delete T;
    
    // now, the STL will destroy these normally
}


// =============================================================================
//      VIRCON C PREPROCESSOR: CONTEXT HANDLING
// =============================================================================


void VirconCPreprocessor::PushContext( VirconCLexer& Lexer )
{
    // create an initial processing context
    ContextStack.emplace_back();
    ProcessingContext& NewContext = ContextStack.back();
    
    // save the reference folder
    NewContext.ReferenceFolder = Lexer.InputDirectory;
    
    // this is safe since there are always start/end tokens
    CTokenList& FirstLine = Lexer.TokenLines.front();
    NewContext.FilePath = FirstLine.front()->Location.FilePath;
    
    // clone all lexer lines into the current context
    for( CTokenList Line: Lexer.TokenLines )
    {
        // insert a new empty line after the last one
        NewContext.SourceLines.emplace_back();
        CTokenList& ClonedList = NewContext.SourceLines.back();
        
        // clone all tokens into the new line
        for( CToken* T: Line )
          ClonedList.push_back( T->Clone() );
    }
    
    // finally initialize iteration
    NewContext.LinePosition = NewContext.SourceLines.begin();
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::PushContext( SourceLocation Location, const std::string& FilePath )
{
    // first, look for the file in the current reference directory
    string PathToInclude = ContextStack.back().ReferenceFolder + PathSeparator + FilePath;
    
    // if not found, then look in the compiler's include directory
    if( !FileExists( PathToInclude ) )
      PathToInclude = CompilerFolder + "include" + PathSeparator + FilePath;
    
    // if not found either, report the error
    if( !FileExists( PathToInclude ) )
      RaiseFatalError( Location, "cannot open include file \"" + FilePath + "\"" );
    
    // tokenize the whole file
    VirconCLexer Lexer;
    Lexer.TokenizeFile( PathToInclude );
    
    // now call the other version of this function
    PushContext( Lexer );
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::PopContext()
{
    if( ContextStack.empty() )
      return;
    
    // cloned token lines are deleted by the destructor
    ContextStack.pop_back();
}


// =============================================================================
//      VIRCON C PREPROCESSOR: FUNCTION-LIKE MACRO EXPANSION
// =============================================================================


// Expands a function-like macro call by replacing the parameter
// names in its body with the call's tokens for each argument.
// Since definitions can use other definitions, this method will
// return an iterator pointing to the start of the inserted
// expansion, so the caller can resume scanning if necessary
CTokenIterator VirconCPreprocessor::ExpandFunctionMacro
(
    CTokenList& Line,               // full line containing the macro call
    CTokenIterator StartPosition,   // position of the macro name in the line
    const FunctionMacro& Macro,     // definition of the called macro
    SourceLocation CallLocation     // location of the call before replacements
)
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Erase the macro name and parenthesis
    // (after this StartPosition points at the first token inside the argument list)
    StartPosition = Line.erase( StartPosition );  // deletes macro name
    StartPosition = Line.erase( StartPosition );  // erases opening parenthesis
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Collect a list of tokens for each argument
    // (it is allowed for any of these lists to be empty)
    vector< CTokenList > Arguments;
    Arguments.emplace_back();   // start the first argument slot
    int ParenthesisDepth = 0;
    
    while( true )
    {
        // running off the end of the line means unterminated call
        if( StartPosition == Line.end() )
          RaiseFatalError( CallLocation, "unterminated function-like macro call (missing closing ')')" );
        
        CToken* T = *StartPosition;
        
        // CASE 1: nested open parenthesis
        if( TokenIsThisDelimiter( T, DelimiterTypes::OpenParenthesis ) )
        {
            ParenthesisDepth++;
            Arguments.back().push_back( T );
            StartPosition = Line.erase( StartPosition );
            continue;
        }
        
        // CASE 2: closing parenthesis
        if( TokenIsThisDelimiter( T, DelimiterTypes::CloseParenthesis ) )
        {
            if( ParenthesisDepth > 0 )
            {
                // still inside a nested call - keep as argument token
                ParenthesisDepth--;
                Arguments.back().push_back( T );
                StartPosition = Line.erase( StartPosition );
                continue;
            }
            
            // this is the closing ')' of the macro call
            StartPosition = Line.erase( StartPosition );
            break;
        }
        
        // CASE 3: a comma at top level separates arguments
        if( TokenIsThisSymbol( T, SpecialSymbolTypes::Comma ) && ParenthesisDepth == 0 )
        {
            Arguments.emplace_back();
            StartPosition = Line.erase( StartPosition );
            continue;
        }
        
        // CASE 4: any other token belongs to the current argument
        Arguments.back().push_back( T );
        StartPosition = Line.erase( StartPosition );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Validate argument count
    // StartPosition now points at the token right after the closing ')'
    
    // Careful! A no-parameter macro call still produces 1 empty argument slot;
    // so normalize that back to zero arguments so the count check passes.
    if( Macro.Parameters.empty() )
      if( Arguments.size() == 1 && Arguments[ 0 ].empty() )
        Arguments.clear();
    
    // now we can check safely
    if( Arguments.size() != Macro.Parameters.size() )
    {
        string Message = "wrong number of arguments in function-like macro call: ";
        Message += "expected " + to_string( Macro.Parameters.size() ) + ", ";
        Message += "received" + to_string( Arguments.size() );
        
        RaiseFatalError( CallLocation, Message );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Expand macro call: walk through the macro body
    // and replace parameter names with their mapped arguments.
    
    // build a helper map to determine parameter indices from names
    map< string, int > ParameterIndices;
    
    for( int i = 0; i < (int)Macro.Parameters.size(); i++ )
      ParameterIndices[ Macro.Parameters[ i ] ] = i;
    
    // we insert all expanded tokens immediately before StartPosition
    // (which points just after the erased call), so the caller's
    // iterator stays valid and re-scanning starts from the expansion.
    CTokenIterator InsertPoint = StartPosition;
    
    for( CToken* BodyToken: Macro.Body )
    {
        // identifiers may be a parameter to replace
        if( BodyToken->Type() == CTokenTypes::Identifier )
        {
            // try to find the parameter index of this name
            string TokenName = ((IdentifierToken*)BodyToken)->Name;
            auto IndexPair = ParameterIndices.find( TokenName );
            
            if( IndexPair != ParameterIndices.end() )
            {
                // substitute: insert clones of the matching argument tokens
                const CTokenList& ArgTokens = Arguments[ IndexPair->second ];
                
                for( CToken* ArgTok: ArgTokens )
                {
                    CToken* Cloned = ArgTok->Clone();
                    Cloned->Location = CallLocation;
                    Line.insert( InsertPoint, Cloned );
                }
                
                continue;
            }
        }
        
        // non-parameter tokens are just cloned and inserted
        CToken* Cloned = BodyToken->Clone();
        Cloned->Location = CallLocation;
        Line.insert( InsertPoint, Cloned );
    }
    
    // Return iterator to the start of the inserted expansion.
    // (if nothing was inserted, InsertPoint already points to the right place)
    // The outer while loop in ReplaceDefinitions may keep re-scanning the line
    return InsertPoint;
}


// =============================================================================
//      VIRCON C PREPROCESSOR: INSERTION FUNCTIONS
// =============================================================================


// This will perform all replacements in place (copy not made).
// Returns true if any replacement was actually made
bool VirconCPreprocessor::ReplaceDefinitions( CTokenList& Line )
{
    CTokenIterator Position = Line.begin();
    bool WereReplacementsMade = false;
    
    // process all tokens in the line
    while( Position != Line.end() )
    {
        CToken* NextToken = *Position;
        SourceLocation OriginalLocation = NextToken->Location;
        
        // operate only on identifiers
        if( NextToken->Type() != CTokenTypes::Identifier )
        {
            Position++;
            continue;
        }
        
        string IdentifierName = ((IdentifierToken*)NextToken)->Name;
        
        // ----------------------------------------------------------------
        // CASE 1: function-like macro
        // ----------------------------------------------------------------
        auto FuncPair = FunctionDefinitions.find( IdentifierName );
        
        if( FuncPair != FunctionDefinitions.end() )
        {
            // expand the macro only if the parenthesis follows the name
            CTokenIterator NextPosition = Next( Position );
            
            if( NextPosition != Line.end() && TokenIsThisDelimiter( *NextPosition, DelimiterTypes::OpenParenthesis ) )
            {
                Position = ExpandFunctionMacro( Line, Position, FuncPair->second, OriginalLocation );
                WereReplacementsMade = true;
                continue;
            }
            
            // in other cases the macro is not expanded and is left as-is
            Position++;
            continue;
        }
        
        // ----------------------------------------------------------------
        // CASE 2: object-like macro
        // ----------------------------------------------------------------
        auto Pair = Definitions.find( IdentifierName );
        
        if( Pair == Definitions.end() )
        {
            Position++;
            continue;
        }
        
        // now, actually replace the name with the definition
        WereReplacementsMade = true;
        CTokenList& ValueTokens = Pair->second;
        
        // (1) remove the identifier (not needed anymore)
        Position = Line.erase( Position );
        
        // (2) The definition value can be composed of several tokens.
        // We need to insert all of them in place of the identifier.
        for( CToken* ValueToken: ValueTokens )
        {
            // we need to clone the token so that
            // the line number can be adjusted!!
            // otherwise parsing will be incorrect
            CToken* ClonedToken = ValueToken->Clone();
            ClonedToken->Location = OriginalLocation;
            
            // now insert it
            Position = Line.insert( Position, ClonedToken );
            Position++;
        }
    }
    
    return WereReplacementsMade;
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::IncludeFile( SourceLocation Location, const string& FilePath )
{
    // create a nested processing context
    PushContext( Location, FilePath );
    
    // (now the preprocessor will process the included content)
}


// =============================================================================
//      VIRCON C PREPROCESSOR: PARSERS FOR DIRECTIVES
// =============================================================================


void VirconCPreprocessor::ProcessLine()
{
    CTokenList& Line = ContextStack.back().GetCurrentLine();
    
    // CASE 1: empty lines are ignored
    // (there shouldn't be any, but be safe)
    if( Line.empty() )
      return;
    
    // CASE 2: avoid having multiple file start/end tokens when including
    if( ContextStack.size() > 1 )
    {
        if( Line.front()->Type() == CTokenTypes::StartOfFile )
          return;
        
        if( Line.front()->Type() == CTokenTypes::EndOfFile )
          return;
    }
    
    // classify current line
    bool LineIsIgnored = !ContextStack.back().AreAllIfConditionsMet();
    bool LineIsDirective = TokenIsThisSymbol( Line.front(), SpecialSymbolTypes::Hash );
    
    // CASE 3: non-directive lines are just cloned and appended to the output
    // (after performing replacements on defined identifiers)
    if( !LineIsDirective )
    {
        if( !LineIsIgnored )
        {
            // macro expansions can themselves use other definitions that also
            // need to be expanded, so keep re-scanning and replacing in each
            // line, up to a configurable number of maximum passes 
            int DefinitionPasses = 0;
            
            while( ReplaceDefinitions( Line ) )
            {
                DefinitionPasses++;
                
                if( DefinitionPasses > MAX_EXPANSION_PASSES )
                  RaiseFatalError( Line.front()->Location, "definition replacement is too deep (possible circular reference)" );
            }
            
            // now copy the replaced line to the output
            for( CToken* T: Line )
              ProcessedTokens.push_back( T->Clone() );
        }
        
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 4: directives
    // empty directives are legal, so check for it
    // (this line is ignored and not copied to the output)
    CTokenIterator TokenPosition = Line.begin();
    TokenPosition++;
    
    if( TokenPosition == Line.end() )
      return;
    
    // detect directive name
    string DirectiveName = ExpectIdentifier( TokenPosition );
    
    // discard ignored directives
    if( LineIsIgnored )
      if( DirectiveName != "ifdef" && DirectiveName != "ifndef"
      &&  DirectiveName != "else" && DirectiveName != "endif" )
        return;
    
    // distinguish the different directives
    if( DirectiveName == "include" )
    {
        // have a depth level limit
        if( ContextStack.size() > 20 )
          RaiseFatalError( Line.front()->Location, "includes are too deeply nested (possible circular references)" );
        
        // now include file contents
        ProcessInclude();
        return;
    }
    
    if( DirectiveName == "define" )
      ProcessDefine();
    
    else if( DirectiveName == "undef" )
      ProcessUndef();
    
    else if( DirectiveName == "ifdef" )
      ProcessIf( false );
    
    else if( DirectiveName == "ifndef" )
      ProcessIf( true );
    
    else if( DirectiveName == "else" )
      ProcessElse();
    
    else if( DirectiveName == "endif" )
      ProcessEndif();
    
    else if( DirectiveName == "error" )
      ProcessError( false );
    
    else if( DirectiveName == "warning" )
      ProcessError( true );
    
    // reject any other directives
    else
      RaiseFatalError( (*Line.begin())->Location, string("unsupported preprocessor directive \"") + DirectiveName + "\"" );
}


// =============================================================================
//      VIRCON C PREPROCESSOR: PROCESSOR FUNCTIONS FOR SPECIFIC DIRECTIVES
// =============================================================================


void VirconCPreprocessor::ProcessInclude()
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect a string literal in the third place
    if( TokenPosition == DirectiveLine.end() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "include file path is missing" );
    
    CToken* FilePathToken = *TokenPosition;
    
    // give a special error message for includes that use
    // brackets (not supported in this compiler)
    if( TokenIsThisOperator( FilePathToken, OperatorTypes::Less ) )
      RaiseFatalError( FilePathToken->Location, "this compiler only supports #include \"path\", not #include <path>" ); 
    
    // otherwise expect a path string
    if( FilePathToken->Type() != CTokenTypes::LiteralString )
      RaiseFatalError( FilePathToken->Location, "expected file path string" );
    
    string IncludedFilePath = ((LiteralStringToken*)FilePathToken)->Value;
    
    // expect an end of line
    TokenPosition++;
    
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
    
    // include the referenced file
    IncludeFile( FilePathToken->Location, IncludedFilePath );
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessDefine()
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect a definition name in the third place
    if( TokenPosition == DirectiveLine.end() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "definition name is missing" );
    
    // consume the name token but save it to determine its column
    CToken* NameToken = *TokenPosition;
    string DefinitionName = ExpectIdentifier( TokenPosition );
    
    // determine if this macro is treated as function or object;
    // for a function-like macro we require that the token '('
    // immediately follows the name with no whitespace in between
    bool IsFunctionMacro = false;
    
    if( TokenPosition != DirectiveLine.end() )
    {
        CToken* NextToken = *TokenPosition;
        
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::OpenParenthesis ) )
        {
            int ColumnAfterName = NameToken->Location.Column + (int)DefinitionName.length();
            
            if( NextToken->Location.Column == ColumnAfterName )
              IsFunctionMacro = true;
        }
    }
    
    // redefinition replaces any previous ones, so before inserting
    // the new definition remove any object-like macro with that name
    auto ObjectPosition = Definitions.find( DefinitionName );
    
    if( ObjectPosition != Definitions.end() )
    {
        for( CToken* T: ObjectPosition->second )
          delete T;
        
        Definitions.erase( ObjectPosition );
    }
    
    // same thing for previously existing function-like definitions
    auto FunctionPosition = FunctionDefinitions.find( DefinitionName );
    
    if( FunctionPosition != FunctionDefinitions.end() )
    {
        for( CToken* T: FunctionPosition->second.Body )
          delete T;
        
        FunctionDefinitions.erase( FunctionPosition );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 1: define a function-like macro
    
    if( IsFunctionMacro )
    {
        FunctionMacro NewMacro;
        
        // consume '('
        TokenPosition++;
        
        // parse parameter list
        if( TokenPosition == DirectiveLine.end() )
          RaiseFatalError( NameToken->Location, "unterminated parameter list in macro definition" );
        
        if( !TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::CloseParenthesis ) )
        {
            // read comma-separated identifiers until ')'
            while( true )
            {
                if( TokenPosition == DirectiveLine.end() )
                  RaiseFatalError( NameToken->Location, "unterminated parameter list in macro definition" );
                
                string ParameterName = ExpectIdentifier( TokenPosition );
                
                // check for duplicate parameter names
                for( const string& Existing: NewMacro.Parameters )
                  if( Existing == ParameterName )
                    RaiseFatalError( (*Previous(TokenPosition))->Location, "duplicate parameter \"" + ParameterName + "\" in macro definition" );
                
                // add a new parameter name
                NewMacro.Parameters.push_back( ParameterName );
                
                // we should have some valid delimiter
                if( TokenPosition == DirectiveLine.end() )
                  RaiseFatalError( NameToken->Location, "unterminated parameter list in macro definition" );
                
                CToken* Delimiter = *TokenPosition;
                
                // for ')' finish parsing parameters
                if( TokenIsThisDelimiter( Delimiter, DelimiterTypes::CloseParenthesis ) )
                {
                    TokenPosition++;
                    break;
                }
                
                // for ',' expect one more parameter 
                if( TokenIsThisSymbol( Delimiter, SpecialSymbolTypes::Comma ) )
                {
                    TokenPosition++;
                    continue;
                }
                
                RaiseFatalError( Delimiter->Location, "expected ',' or ')' in macro parameter list" );
            }
        }
        
        // Collect body tokens until end of line
        while( TokenPosition != DirectiveLine.end() )
        {
            CToken* BodyToken = *TokenPosition;
            
            // definitions cannot contain the hash symbol (not supported)
            if( TokenIsThisSymbol( BodyToken, SpecialSymbolTypes::Hash ) )
              RaiseFatalError( BodyToken->Location, "definitions cannot contain the hash symbol (#)" );
            
            NewMacro.Body.push_back( BodyToken->Clone() );
            TokenPosition++;
        }
        
        FunctionDefinitions[ DefinitionName ] = NewMacro;
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 2: define an object-like macro
    
    // create a new definition
    CTokenList EmptyList;
    Definitions[ DefinitionName ] = EmptyList;
    CTokenList& DefinitionValueTokens = Definitions[ DefinitionName ];
    
    // keep adding tokens to the definition until end of line
    while( TokenPosition != DirectiveLine.end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // check that the definition does not use itself
        if( NextToken->Type() == CTokenTypes::Identifier )
          if( ((IdentifierToken*)NextToken)->Name == DefinitionName )
            RaiseFatalError( NextToken->Location, "a definition cannot contain itself (circular reference)" );
        
        // definitions cannot contain the hash symbol (not supported)
        if( TokenIsThisSymbol( NextToken, SpecialSymbolTypes::Hash ))
          RaiseFatalError( NextToken->Location, "definitions cannot contain the hash symbol (#)" );
        
        // now we can safely copy the token to the definition
        DefinitionValueTokens.push_back( NextToken->Clone() );
        
        // advance to next token
        TokenPosition++;
    }
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessUndef()
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect a definition name in the third place
    if( TokenPosition == DirectiveLine.end() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "definition name is missing" );
    
    string DefinitionName = ExpectIdentifier( TokenPosition );
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
      
    // remove from object-like macros if it existed there
    auto ObjectPosition = Definitions.find( DefinitionName );
    
    if( ObjectPosition != Definitions.end() )
    {
        for( CToken* T: ObjectPosition->second )
          delete T;
        
        Definitions.erase( ObjectPosition );
    }
    
    // also remove from function-like macros if it existed there
    auto FunctionPosition = FunctionDefinitions.find( DefinitionName );
    
    if( FunctionPosition != FunctionDefinitions.end() )
    {
        for( CToken* T: FunctionPosition->second.Body )
          delete T;
        
        FunctionDefinitions.erase( FunctionPosition );
    }
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessIf( bool IsIfndef )
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect an identifier in the third place
    if( TokenPosition == DirectiveLine.end() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "expected an identifier" );
    
    string QueriedDefinition = ExpectIdentifier( TokenPosition );
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
    
    // create a new if context
    ContextStack.back().IfStack.emplace_back();
    IfContext& AddedContext = ContextStack.back().IfStack.back();
    AddedContext.StartingLine = (*DirectiveLine.begin())->Location.Line;
    AddedContext.ElseWasFound = false;
    
    // check the condition: the name is defined if it exists in either map
    bool DefinitionExists =
        (Definitions.find( QueriedDefinition ) != Definitions.end()) ||
        (FunctionDefinitions.find( QueriedDefinition ) != FunctionDefinitions.end());
    
    AddedContext.ConditionIsMet = (DefinitionExists == !IsIfndef);
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessElse()
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
      
    // there needs to be some active #if
    if( ContextStack.back().IfStack.empty() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "#else with no previous #if" );
    
    // there cannot be more than 1 #else
    if( ContextStack.back().IfStack.back().ElseWasFound )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "#else can only be used once per #if" );
    
    // apply the directive
    ContextStack.back().IfStack.back().ElseWasFound = true;
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessEndif()
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
      
    // there needs to be some active #if
    if( ContextStack.back().IfStack.empty() )
      RaiseFatalError( (*DirectiveLine.begin())->Location, "#endif with no previous #if" );
    
    // apply the directive
    ContextStack.back().IfStack.pop_back();
}

// -----------------------------------------------------------------------------

void VirconCPreprocessor::ProcessError( bool WarningOnly )
{
    CTokenList& DirectiveLine = ContextStack.back().GetCurrentLine();
    CTokenIterator TokenPosition = DirectiveLine.begin();
    advance( TokenPosition, 2 );
    
    // expect a message string
    if( TokenPosition == DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected a string" );
      
    if( (*TokenPosition)->Type() != CTokenTypes::LiteralString )
      RaiseFatalError( (*TokenPosition)->Location, "expected a string" );
    
    LiteralStringToken* MessageToken = (LiteralStringToken*)(*TokenPosition);
    string Message = MessageToken->Value;
    TokenPosition++;
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
      
    // raise the error/warning
    if( WarningOnly )  RaiseWarning   ( (*DirectiveLine.begin())->Location, Message );
    else               RaiseFatalError( (*DirectiveLine.begin())->Location, Message );
}


// =============================================================================
//      VIRCON C PREPROCESSOR: PROCESSING IDENTIFIERS
// =============================================================================


void VirconCPreprocessor::RecognizeKeywords()
{
    for( CToken* &T: ProcessedTokens )
    {
        // operate only on identifiers
        if( T->Type() != CTokenTypes::Identifier )
          continue;
        
        // for non-keywords, keep the token as it is
        string IdentifierName = ((IdentifierToken*)T)->Name;
        
        if( !IsKeyword( IdentifierName ) )
          continue;
        
        // replace the identifier with the equivalent keyword
        KeywordToken* NewToken = NewKeywordToken( T->Location, WhichKeyword( IdentifierName ) );
        delete T;
        T = NewToken;
    }
}


// =============================================================================
//      VIRCON C PREPROCESSOR: MAIN PROCESSING FUNCTION
// =============================================================================


void VirconCPreprocessor::Preprocess( VirconCLexer& Lexer )
{
    // delete any previous parsing state
    while( !ContextStack.empty() )
      PopContext();
    
    // delete any previous object-like macro definitions
    Definitions.clear();
    
    // delete any previous function-like macro definitions
    for( auto& Pair: FunctionDefinitions )
      for( CToken* T: Pair.second.Body )
        delete T;
    
    FunctionDefinitions.clear();
    
    // delete any previous results
    for( CToken* T : ProcessedTokens )
      delete T;
    
    ProcessedTokens.clear();
    
    // create an initial processing context
    PushContext( Lexer );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now process all cloned source lines, one at a time
    // (process functions will update this iterator)
    while( !ContextStack.empty() )
    {
        while( !ContextStack.back().LinesHaveEnded() )
        {
            ProcessingContext& ContextBeingProcessed = ContextStack.back();
            ProcessLine();
            
            // careful! do it this way, otherwise if processing
            // changes the context we may get incorrect iterations
            ContextBeingProcessed.Advance();
        }
        
        PopContext();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // after preprocessing, parser needs to have keywords
    // and identifiers separately (so far no keywords existed)
    RecognizeKeywords();
}
