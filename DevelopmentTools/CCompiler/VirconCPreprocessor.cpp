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
        
        // search this name in the defined values
        string IdentifierName = ((IdentifierToken*)NextToken)->Name;
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
            Position = ProcessedTokens.insert( Position, ClonedToken );
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
            // keep replacing: definitions can use other definitions
            int DefinitionCycles = 0;
            
            while( ReplaceDefinitions( Line ) )
            {
                DefinitionCycles++;
                
                if( DefinitionCycles > 10 )
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
    
    string DefinitionName = ExpectIdentifier( TokenPosition );
    
    // create a new definition
    CTokenList EmptyList;
    Definitions[ DefinitionName ] = EmptyList;
    CTokenList& DefinitionValueTokens = Definitions[ DefinitionName ];
    
    // keep adding tokens to the definition
    // until we find the end of line line
    CToken* NextToken = *TokenPosition;
    
    while( TokenPosition != DirectiveLine.end() )
    {
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
        NextToken = *TokenPosition;
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
      
    // now remove the definition, if it existed
    auto Pair = Definitions.find( DefinitionName );
    
    if( Pair != Definitions.end() )
    {
        // we need to delete all cloned tokens
        for( CToken* T: Pair->second )
          delete T;
        
        Definitions.erase( Pair );
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
    
    // check the condition
    auto Pair = Definitions.find( QueriedDefinition );
    bool DefinitionExists = (Pair != Definitions.end());
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
    
    // expect an end of line
    if( TokenPosition != DirectiveLine.end() )
      RaiseFatalError( (*TokenPosition)->Location, "expected end of line" );
      
    // raise the error/warning
    if( WarningOnly )  RaiseWarning   ( (*DirectiveLine.begin())->Location, Message );
    else               RaiseFatalError( (*DirectiveLine.begin())->Location, Message );
}


// =============================================================================
//      VIRCON C PREPROCESSOR: XXX
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
    
    Definitions.clear();
    
    // delete any previous results
    for( CToken* T : ProcessedTokens )
      delete T;
    
    ProcessedTokens.clear();
    
    // create an initial processing context
    PushContext( Lexer );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now process all clones source lines, one at a time
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
