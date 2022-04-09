// *****************************************************************************
    // include project headers
    #include "VirconCParser.hpp"
    #include "VirconCPreprocessor.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      TOKEN CLASSIFICATION
// =============================================================================


bool IsValidStartOfExpression( CToken* T )
{
    if( IsLastToken( T ) )
      return false;
    
    if( T->Type() == CTokenTypes::Keyword )
    {
        if( ((KeywordToken*)T)->Which == KeywordTypes::SizeOf )  return true;
        if( ((KeywordToken*)T)->Which == KeywordTypes::Null   )  return true;
        return false;
    }
    
    if( T->Type() == CTokenTypes::SpecialSymbol )
      return false;
    
    if( T->Type() == CTokenTypes::Delimiter )
      return ((DelimiterToken*)T)->Which == DelimiterTypes::OpenParenthesis;
    
    if( T->Type() == CTokenTypes::Operator )
      return ((OperatorToken*)T)->IsUnary();
    
    // all other cases are valid
    return true;
}

// -----------------------------------------------------------------------------

bool IsValidStartOfStatement( CToken* T )
{
    if( IsLastToken( T ) )
      return false;
    
    if( T->Type() == CTokenTypes::Keyword )
      return ((KeywordToken*)T)->Which != KeywordTypes::Else;
    
    if( T->Type() == CTokenTypes::Delimiter )
    {
        DelimiterTypes Which = ((DelimiterToken*)T)->Which;
        return (Which == DelimiterTypes::OpenBrace || Which == DelimiterTypes::OpenParenthesis);
    }
    
    if( T->Type() == CTokenTypes::SpecialSymbol )
    {
        SpecialSymbolTypes Which = ((SpecialSymbolToken*)T)->Which;
        return (Which == SpecialSymbolTypes::Semicolon);
    }
    
    // the only other possible statements
    // left now are expressions
    return IsValidStartOfExpression( T );
}

// -----------------------------------------------------------------------------

bool IsValidStartOfType( CToken* T, CNode* Parent )
{
    // these are always valid, regardless of the scope context
    if( TokenIsTypeKeyword( T ) )
      return true;
    
    // these are never valid, regardless of the scope context
    if( T->Type() != CTokenTypes::Identifier )
      return false;
    
    // obtain identifier name
    string SymbolName = ((IdentifierToken*)T)->Name;
    
    // find the applicable scope
    ScopeNode* OwnerScope = Parent->FindClosestScope( Parent );
    
    // with that scope, attempt to resolve it as a type
    CNode* Declaration = OwnerScope->ResolveIdentifier( SymbolName );
    return (Declaration && Declaration->IsType());
}


// =============================================================================
//      VIRCON C PARSER: INSTANCE HANDLING
// =============================================================================


VirconCParser::VirconCParser()
{
    ProgramAST = nullptr;
    Tokens = nullptr;
}

// -----------------------------------------------------------------------------

VirconCParser::~VirconCParser()
{
    delete ProgramAST;
    ProgramAST = nullptr;
    Tokens = nullptr;
}


// =============================================================================
//      VIRCON C PARSER: PARSERS FOR STATEMENTS
// =============================================================================


CNode* VirconCParser::ParseStatement( CNode* Parent, CTokenIterator& TokenPosition, bool IsTopLevel  )
{
    CToken* NextToken = *TokenPosition;
    
    // first cover all invalid cases
    if( IsLastToken(NextToken) )
      RaiseFatalError( NextToken->Location, "unexpected end of file (expected a statement)" );
    
    if( !IsValidStartOfStatement( NextToken ) )
      RaiseFatalError( NextToken->Location, "invalid start of statement: " + NextToken->ToString() );
    
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Embedded ) )
      RaiseFatalError( NextToken->Location, "embedded files can only be declared at the top level" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now choose from all valid cases
    
    // CASE 1: Empty statements
    if( TokenIsThisSymbol( NextToken, SpecialSymbolTypes::Semicolon ) )
    {
        // create it manually since there is no parser function for it
        EmptyStatementNode* EmptyStatement = new EmptyStatementNode( Parent );
        EmptyStatement->Location = NextToken->Location;
        TokenPosition++;

        return EmptyStatement;
    }
    
    // CASE 2: Variable & function declarations
    if( IsValidStartOfType( NextToken, Parent ) )
      return ParseDeclaration( Parent, TokenPosition, IsTopLevel );
    
    // CASE 3: Type declarations
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Struct ))
      return ParseStructure( Parent, TokenPosition );
    
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Union ))
      return ParseUnion( Parent, TokenPosition );
    
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Enum ))
      return ParseEnumeration( Parent, TokenPosition ); 
    
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Typedef ))
      return ParseTypedef( Parent, TokenPosition );
      
    // CASE 4: Statements exclusive to functions
    if( NextToken->Type() == CTokenTypes::Keyword )
    {
        KeywordToken* Keyword = (KeywordToken*)NextToken;
        
        switch( Keyword->Which )
        {
            case KeywordTypes::If:
                return ParseIf( Parent, TokenPosition );
                
            case KeywordTypes::While:
                return ParseWhile( Parent, TokenPosition );
                
            case KeywordTypes::Do:
                return ParseDo( Parent, TokenPosition );
                
            case KeywordTypes::For:
                return ParseFor( Parent, TokenPosition );
                
            case KeywordTypes::Return:
                return ParseReturn( Parent, TokenPosition );
                
            case KeywordTypes::Break:
                return ParseBreak( Parent, TokenPosition );
                
            case KeywordTypes::Continue:
                return ParseContinue( Parent, TokenPosition );
                
            case KeywordTypes::Switch:
                return ParseSwitch( Parent, TokenPosition );
                
            case KeywordTypes::Case:
                return ParseCase( Parent, TokenPosition );
                
            case KeywordTypes::Default:
                return ParseDefault( Parent, TokenPosition );
                
            case KeywordTypes::Goto:
                return ParseGoto( Parent, TokenPosition );
                
            case KeywordTypes::Asm:
                return ParseAssemblyBlock( Parent, TokenPosition );
                
            default:
                break;
        }
    }
    
    // CASE 5: Blocks
    if( TokenIsThisDelimiter( NextToken, DelimiterTypes::OpenBrace ) )
    {
        TokenPosition++;
        return ParseBlock( Parent, TokenPosition );
    }
    
    // CASE 6: An identifier is a label if followed by a colon
    if( NextToken->Type() == CTokenTypes::Identifier )
    {
        CToken* TokenAfterNext = *Next( TokenPosition );
        
        if( TokenIsThisSymbol( TokenAfterNext, SpecialSymbolTypes::Colon ) )
          return ParseLabel( Parent, TokenPosition );
    }
    
    // CASE 7: give special error messages for non supported features of standard C
    if( NextToken->Type() == CTokenTypes::Identifier )
    {
        IdentifierToken* IT = (IdentifierToken*)NextToken;
        
        if( IT->Name == "const" )
          RaiseFatalError( NextToken->Location, "const variables are not supported in this compiler" );
    }
    
    // CASE 8: Any other case must be an expression.
    // Parse it normally but expect a semicolon after it
    CNode* Expression = ParseExpression( Parent, TokenPosition );
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    return Expression;
}

// -----------------------------------------------------------------------------

// expects the opening brace to be already consumed
BlockNode* VirconCParser::ParseBlock( CNode* Parent, CTokenIterator& TokenPosition )
{
    BlockNode* Block = new BlockNode( Parent );
    Block->Location = (*TokenPosition)->Location;
    
    // keep parsing
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( Block->Location, "block is not closed (unexpected end of file)" );
        
        // CASE 2: end of block
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: statement
        CNode* Statement = ParseStatement( Block, TokenPosition, false );
        Block->Statements.push_back( Statement );
    }
    
    return Block;
}

// -----------------------------------------------------------------------------

AssemblyBlockNode* VirconCParser::ParseAssemblyBlock( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "asm"
    AssemblyBlockNode* AssemblyBlock = new AssemblyBlockNode( Parent );
    AssemblyBlock->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // expect open brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing text strings until brace is closed
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // detect end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( AssemblyBlock->Location, "assembly block is not closed (unexpected end of file)" );
        
        // detect closing brace
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ) )
        {
            TokenPosition++;
            break;
        }
        
        // otherwise expect only text strings
        if( NextToken->Type() != CTokenTypes::LiteralString )
          RaiseFatalError( NextToken->Location, "assembly blocks can only contain text strings" );
        
        // create a new assembly line
        string LineText = ((LiteralStringToken*)NextToken)->Value;
        
        AssemblyBlock->AssemblyLines.emplace_back();
        AssemblyBlockNode::AssemblyLine& AddedLine = AssemblyBlock->AssemblyLines.back();
        AddedLine.Text = LineText;
        AddedLine.EmbeddedAtom = nullptr;
        
        // check if there is an embedded atom
        unsigned OpenBracePosition = LineText.find( '{' );
        
        if( OpenBracePosition != string::npos )
        {
            // fint closing brace
            unsigned CloseBracePosition = LineText.find( '}', OpenBracePosition );
            
            if( CloseBracePosition == string::npos )
            {
                RaiseError( NextToken->Location, "brace in assembly string is not closed" );
                TokenPosition++;
                continue;
            }
            
            // extract the delimited variable name
            unsigned Length = (CloseBracePosition - OpenBracePosition + 1);
            string VariableName = LineText.substr( OpenBracePosition+1, Length-2 );
            
            // create an atom with that variable
            AddedLine.EmbeddedAtom = new ExpressionAtomNode( AssemblyBlock );
            AddedLine.EmbeddedAtom->Location = NextToken->Location;
            AddedLine.EmbeddedAtom->IdentifierName = VariableName;
            AddedLine.EmbeddedAtom->ResolveIdentifier();
            
            // only variables are allowed
            if( AddedLine.EmbeddedAtom->AtomType != AtomTypes::Variable )
              RaiseError( NextToken->Location, "braces in assembly only accept variables" );
            
            // referenced variable must be size 1
            if( AddedLine.EmbeddedAtom->ResolvedVariable->DeclaredType->SizeInWords() != 1 )
              RaiseError( NextToken->Location, "variable in braces must be of size 1" );
        }

        // add the new string as a line and continue
        TokenPosition++;
    }
    
    return AssemblyBlock;
}

// -----------------------------------------------------------------------------

IfNode* VirconCParser::ParseIf( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "if"
    IfNode* NewIf = new IfNode( Parent );
    NewIf->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse the condition between parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    NewIf->Condition = ParseExpression( NewIf, TokenPosition );
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    
    // parse the "true" statement
    NewIf->TrueStatement = ParseStatement( NewIf, TokenPosition, false );
    
    // optionally, parse a "false" statement
    if( TokenIsThisKeyword( *TokenPosition, KeywordTypes::Else ) )
    {
        TokenPosition++;
        NewIf->FalseStatement = ParseStatement( NewIf, TokenPosition, false );
    }
    
    return NewIf;
}

// -----------------------------------------------------------------------------

WhileNode* VirconCParser::ParseWhile( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "while"
    WhileNode* NewWhile = new WhileNode( Parent );
    NewWhile->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse the condition between parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    NewWhile->Condition = ParseExpression( NewWhile, TokenPosition );
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    
    // parse the looped statement
    NewWhile->LoopStatement = ParseStatement( NewWhile, TokenPosition, false );
    return NewWhile;
}

// -----------------------------------------------------------------------------

DoNode* VirconCParser::ParseDo( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "do"
    DoNode* NewDo = new DoNode( Parent );
    NewDo->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse the looped statement
    NewDo->LoopStatement = ParseStatement( NewDo, TokenPosition, false );
    
    // expect "while"
    ExpectKeyword( TokenPosition, KeywordTypes::While );
    
    // parse the condition between parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    NewDo->Condition = ParseExpression( NewDo, TokenPosition );
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    
    // expect a semicolon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    return NewDo;
}

// -----------------------------------------------------------------------------

ForNode* VirconCParser::ParseFor( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "for"
    ForNode* NewFor = new ForNode( Parent );
    NewFor->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // expect open parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    
    // initial action is optional, so check if it exists
    if( TokenIsThisSymbol( *TokenPosition, SpecialSymbolTypes::Semicolon ) )
      TokenPosition++;
    
    else
    {
        // parse initial action (must be either
        // a variable declaration or an expression
        if( TokenIsTypeKeyword( *TokenPosition ) )
          NewFor->InitialAction = ParseDeclaration( NewFor, TokenPosition, false );
        else
          NewFor->InitialAction = ParseStatement( NewFor, TokenPosition, false );
    }
    
    // condition is optional, so check if it exists
    if( TokenIsThisSymbol( *TokenPosition, SpecialSymbolTypes::Semicolon ) )
      TokenPosition++;
    
    else
    {
        // parse condition expression
        NewFor->Condition = ParseExpression( NewFor, TokenPosition, true );
        
        // expect a semicolon as separator
        ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );        
    }
    
    // iteration action is optional, so check if it exists
    if( !TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::CloseParenthesis ) )
      NewFor->IterationAction = ParseExpression( NewFor, TokenPosition, true );
    
    // in any case, expect close parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    
    // now parse the body statement
    NewFor->LoopStatement = ParseStatement( NewFor, TokenPosition, false );
    
    return NewFor;
}

// -----------------------------------------------------------------------------

ReturnNode* VirconCParser::ParseReturn( CNode* Parent, CTokenIterator& TokenPosition )
{
    ReturnNode* NewReturn = new ReturnNode( Parent );
    NewReturn->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // we may optionally have a returned expression
    if( IsValidStartOfExpression( *TokenPosition ) )
      NewReturn->ReturnedExpression = ParseExpression( NewReturn, TokenPosition );
    
    // in either case, expect a semicolon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // before returning, resolve the function context for this Return
    NewReturn->ResolveContext();
    return NewReturn;
}

// -----------------------------------------------------------------------------

BreakNode* VirconCParser::ParseBreak( CNode* Parent, CTokenIterator& TokenPosition )
{
    BreakNode* NewBreak = new BreakNode( Parent );
    NewBreak->Location = (*TokenPosition)->Location;
    
    // expect semicolon
    TokenPosition++;
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // before returning, resolve the loop or switch context for this Break
    NewBreak->ResolveContext();
    return NewBreak;
}

// -----------------------------------------------------------------------------

ContinueNode* VirconCParser::ParseContinue( CNode* Parent, CTokenIterator& TokenPosition )
{
    ContinueNode* NewContinue = new ContinueNode( Parent );
    NewContinue->Location = (*TokenPosition)->Location;
    
    // expect semicolon
    TokenPosition++;
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // before returning, resolve the loop context for this Continue
    NewContinue->ResolveContext();
    return NewContinue;
}

// -----------------------------------------------------------------------------

SwitchNode* VirconCParser::ParseSwitch( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "switch"
    SwitchNode* NewSwitch = new SwitchNode( Parent );
    NewSwitch->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse the condition between parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    NewSwitch->Condition = ParseExpression( NewSwitch, TokenPosition );
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    
    // expect open brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing body statements
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( NewSwitch->Location, "switch is not closed (unexpected end of file)" );
        
        // CASE 2: end of block
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: statement
        CNode* Statement = ParseStatement( NewSwitch, TokenPosition, false );
        NewSwitch->Statements.push_back( Statement );
        
        // for a switch, we must consider errors variables
        // declared after cases exist (only when direct children)
        if( Statement->Type() == CNodeTypes::VariableList )
          if( !NewSwitch->HandledCases.empty() || NewSwitch->DefaultCase )
            RaiseError( Statement->Location, "variables cannot be declared in a switch after case/default are used" );
    }
    
    return NewSwitch;
}

// -----------------------------------------------------------------------------

CaseNode* VirconCParser::ParseCase( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "case"
    CaseNode* NewCase = new CaseNode( Parent );
    NewCase->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse a case value
    NewCase->ValueExpression = ParseExpression( NewCase, TokenPosition );
    
    // expect a colon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Colon );
    
    // before returning, resolve the switch context for this case
    NewCase->ResolveContext();
    return NewCase;
}

// -----------------------------------------------------------------------------

DefaultNode* VirconCParser::ParseDefault( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "default"
    DefaultNode* NewDefault = new DefaultNode( Parent );
    NewDefault->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // expect a colon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Colon );
    
    // before returning, resolve the switch context for this default
    NewDefault->ResolveContext();
    return NewDefault;
}

// -----------------------------------------------------------------------------

LabelNode* VirconCParser::ParseLabel( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node
    LabelNode* NewLabel = new LabelNode( Parent );
    NewLabel->Location = (*TokenPosition)->Location;
    
    // here, we already know an identifier is next
    NewLabel->Name = ExpectIdentifier( TokenPosition );
    
    // expect a colon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Colon );
    
    // after the label itself there should be an expression (not
    // really needed here, but check it to conform with C standard)
    if( !IsValidStartOfStatement( *TokenPosition ) )
      RaiseFatalError( (*TokenPosition)->Location, "expected statement after label" );
    
    // before returning, resolve the function context for this label
    NewLabel->ResolveContext();
    return NewLabel;
}

// -----------------------------------------------------------------------------

GotoNode* VirconCParser::ParseGoto( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create the node, and consume "goto"
    GotoNode* NewGoto = new GotoNode( Parent );
    NewGoto->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // read the label name
    NewGoto->LabelName = ExpectIdentifier( TokenPosition );
    
    // expect a semicolon as end
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // before returning, resolve the function context for this goto
    NewGoto->ResolveContext();
    return NewGoto;
}


// =============================================================================
//      VIRCON C PARSER: PARSERS FOR DECLARATIONS
// =============================================================================


DataType* VirconCParser::ParseType( CNode* Parent, CTokenIterator& TokenPosition )
{
    // first, read the base type
    DataType* ParsedType = nullptr;
    CToken* FirstToken = *TokenPosition;
    
    // case 1: type keyword
    if( FirstToken->Type() == CTokenTypes::Keyword )
    {
        switch( ((KeywordToken*)FirstToken)->Which )
        {
            case KeywordTypes::Int:
                ParsedType = new PrimitiveType( PrimitiveTypes::Int );
                break;
                
            case KeywordTypes::Float:
                ParsedType = new PrimitiveType( PrimitiveTypes::Float );
                break;
                
            case KeywordTypes::Bool:
                ParsedType = new PrimitiveType( PrimitiveTypes::Bool );
                break;
                
            case KeywordTypes::Void:
                ParsedType = new VoidType();
                break;
            
            default:
                RaiseFatalError( FirstToken->Location, "expected a type" );
        }
        
        // consume the keyword
        TokenPosition++;
    }
    
    // case 2: identifier defined as a type
    else
    {
        // obtain identifier name
        if( FirstToken->Type() != CTokenTypes::Identifier )
          RaiseFatalError( FirstToken->Location, "expected a type" );
        
        string SymbolName = ExpectIdentifier( TokenPosition );
        
        // find the applicable scope
        ScopeNode* OwnerScope = Parent->FindClosestScope( Parent );
        
        // use the scope to resolve it as a type
        CNode* Declaration = OwnerScope->ResolveIdentifier( SymbolName );
        
        if( !Declaration || !Declaration->IsType() )
          RaiseFatalError( FirstToken->Location, "identifier \"" + SymbolName + "\" is not a type" );
        
        // extract type from declaration
        ParsedType = ((TypeNode*)Declaration)->DeclaredType->Clone();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // keep reading pointer and array modifiers
    while( (*TokenPosition)->Type() != CTokenTypes::EndOfFile )
    {
        // read a pointer modifier
        if( TokenIsThisOperator( *TokenPosition, OperatorTypes::Asterisk ) )
        {
            TokenPosition++;
            ParsedType = new PointerType( ParsedType );
        }
        
        // read an array modifier
        else if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBracket ) )
        {
            // array indices are read from left to right, not the
            // usual right to left! So we need to first read all
            // of them and then traverse the dimensions in the list
            list< int > Dimensions;
            
            while( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBracket ) )
            {
                // parse internal expression
                TokenPosition++;
                ExpressionNode* DimensionExpression = ParseExpression( Parent, TokenPosition, true );
                
                // expect closing bracket
                ExpectDelimiter( TokenPosition, DelimiterTypes::CloseBracket );
                
                // variable length arrays are not allowed
                if( !DimensionExpression->IsStatic() )
                  RaiseFatalError( DimensionExpression->Location, "variable length arrays are not allowed" );
                
                StaticValue DimensionValue = DimensionExpression->GetStaticValue();
                
                if( DimensionValue.Type == PrimitiveTypes::Float )
                  RaiseError( DimensionExpression->Location, "array length must be an integer" );
                
                int DimensionLength = DimensionValue.Word.AsInteger;
                
                if( DimensionLength < 1 )
                  RaiseError( DimensionExpression->Location, "array length must be greater than 0" );
                
                // add the dimension at the beginning of the
                // list so that the final order is reversed
                Dimensions.push_front( DimensionLength );
            }
            
            // now keep building arrays in the usual order
            // (as if they had been found right to left)
            for( int Dimension: Dimensions )
              ParsedType = new ArrayType( ParsedType, Dimension );
        }
        
        else break;
    }
    
    return ParsedType;
}

// -----------------------------------------------------------------------------

CNode* VirconCParser::ParseDeclaration( CNode* Parent, CTokenIterator& TokenPosition, bool IsTopLevel )
{
    // first, read the type
    DataType* InitialType = ParseType( Parent, TokenPosition );
    
    // read the name
    string DeclarationName = ExpectIdentifier( TokenPosition );
    
    // functions are followed by parehthesis
    CToken* NextToken = *TokenPosition;
    
    if( TokenIsThisDelimiter( NextToken, DelimiterTypes::OpenParenthesis ) )
    {
        // functions are only allowed at top level
        if( IsTopLevel )
          return ParseFunction( InitialType, DeclarationName, Parent, TokenPosition );
          
        else
          RaiseFatalError( NextToken->Location, "functions cannot be declared inside other functions" );
    }
    
    // any other declaration is a variable
    return ParseVariableList( InitialType, DeclarationName, Parent, TokenPosition );
}

// -----------------------------------------------------------------------------

VariableNode* VirconCParser::ParseFunctionArgument( FunctionNode* Function, CTokenIterator& TokenPosition )
{
    VariableNode* NewArgument = new VariableNode( Function );
    NewArgument->Location = (*TokenPosition)->Location;
    
    // expect a type
    CToken* NextToken = *TokenPosition;
    
    if( IsLastToken( NextToken ) )
      RaiseFatalError( NextToken->Location, "unexpected end of file" );
    
    if( !IsValidStartOfType( NextToken, Function ) )
      RaiseFatalError( NextToken->Location, "expected an argument type" );
    
    NewArgument->DeclaredType = ParseType( Function, TokenPosition );
    
    // function parameters cannot be declared void
    if( NewArgument->DeclaredType->Type() == DataTypes::Void )
      RaiseError( NextToken->Location, "function argument cannot be void" );
       
    // restriction: function parameters cannot have size > 1
    if( NewArgument->DeclaredType->SizeInWords() > 1 )
      RaiseError( NextToken->Location, "functions cannot pass arguments of size > 1" );
    
    // expect only an argument name
    // (in this case, no values are allowed)
    NewArgument->Name = ExpectIdentifier( TokenPosition );
    
    // allocate the argument before returning it
    NewArgument->AllocateAsArgument();
    return NewArgument;
}

// -----------------------------------------------------------------------------

void VirconCParser::ParseFunctionBody( FunctionNode* Function, CTokenIterator& TokenPosition )
{
    // capture start location
    SourceLocation StartLocation = (*TokenPosition)->Location;
    
    // expect the opening brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing until the closing brace
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( StartLocation, "function body not closed (unexpected end of file)" );
          
        // CASE 2: end of block
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: statement
        Function->Statements.push_back( ParseStatement( Function, TokenPosition, false ) );
    }
}

// -----------------------------------------------------------------------------

// expects function type and name to be already consumed
FunctionNode* VirconCParser::ParseFunction( DataType* ReturnType, const string& Name, CNode* Parent, CTokenIterator& TokenPosition )
{
    // type and name have already been parsed and consumed
    FunctionNode* NewFunction = new FunctionNode( Parent );
    NewFunction->Location = (*TokenPosition)->Location;
    NewFunction->Name = Name;
    NewFunction->ReturnType = ReturnType;  // no need to clone (first use)
    
    // functions can only return data of size 1
    if( ReturnType->SizeInWords() > 1 )
      RaiseError( (*TokenPosition)->Location, "functions cannot return values of size > 1" );
    
    // here we already know an open parenthesis is next
    TokenPosition++;
    
    // careful! "No parameters" can be stated by either
    // function(), or function( void ). This is a special case
    CToken* NextToken = *TokenPosition;
    CTokenIterator NextPosition = Next( TokenPosition );
    
    if( TokenIsThisKeyword( NextToken, KeywordTypes::Void )
    &&  TokenIsThisDelimiter( *NextPosition, DelimiterTypes::CloseParenthesis ) )
    {
        // consume "void" and ")"
        TokenPosition++;
        TokenPosition++;
        
        // do NOT parse arguments
        // (leave the list empty)
    }
    
    // keep consuming arguments until a closing parenthesis is found
    else while( !IsLastToken( *TokenPosition ) )
    {
        NextToken = *TokenPosition;
        
        // keep adding arguments until
        // we find a closing parenthesis
        if( NextToken->Type() == CTokenTypes::Delimiter )
          if( ((DelimiterToken*)NextToken)->Which == DelimiterTypes::CloseParenthesis )
          {
              TokenPosition++;
              break;
          }
        
        // if this is not the first argument,
        // expect a comma as separation
        if( !NewFunction->Arguments.empty() )
          ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Comma );
        
        // expect a declaration as next argument
        NewFunction->Arguments.push_back( ParseFunctionArgument( NewFunction, TokenPosition ) );
    }
    
    // before parsing the body, allocate the function name
    // (if we do it later, recursion will not be possible)
    NewFunction->AllocateName();
    
    // now parse function body as a special block
    ParseFunctionBody( NewFunction, TokenPosition );
    
    // update locals allocation
    if( NewFunction->StackSizeForVariables < NewFunction->LocalVariablesSize )
      NewFunction->StackSizeForVariables = NewFunction->LocalVariablesSize;
    
    return NewFunction;
}

// -----------------------------------------------------------------------------

VariableListNode* VirconCParser::ParseVariableList( DataType* DeclaredType, const string& Name, CNode* Parent, CTokenIterator& TokenPosition )
{
    VariableListNode* VariableList = new VariableListNode( Parent );
    VariableList->DeclaredType = DeclaredType;     // no need to clone (first use)
    VariableList->Location = (*TokenPosition)->Location;
    
    // type and name have already been parsed and consumed
    VariableNode* NewVariable = new VariableNode( VariableList );
    NewVariable->Location = (*TokenPosition)->Location;
    NewVariable->DeclaredType = DeclaredType->Clone();
    NewVariable->Name = Name;
    
    // special error report for standard C arrays
    if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBracket ) )
      RaiseFatalError( (*TokenPosition)->Location, "in this compiler, arrays need to be declared as 'type[n] name', not 'type name[n]'" );
    
    // optionally, an assignment can be made on declaration
    if( TokenIsThisOperator( *TokenPosition, OperatorTypes::Equal ) )
    {
        // consume equal sign
        TokenPosition++;
        
        // case 1: initialization list
        if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBrace ) )
          NewVariable->InitialValue = ParseInitializationList( NewVariable, TokenPosition );
        
        // case 2: expression
        else
          NewVariable->InitialValue = ParseExpression( NewVariable, TokenPosition );
    }
    
    // initial variable has been parsed
    NewVariable->AllocateAsVariable();
    VariableList->Variables.push_back( NewVariable );
    
    // there can be multiple declarations of the
    // same type together, separated by commas
    while( !IsLastToken( *TokenPosition ) )
    {
        if( !TokenIsThisSymbol( *TokenPosition, SpecialSymbolTypes::Comma ) )
          break;
    
        // consume the comma
        TokenPosition++;
        
        // parse another variable
        VariableNode* AddedVariable = new VariableNode( VariableList );
        AddedVariable->Location = (*TokenPosition)->Location;
        AddedVariable->DeclaredType = DeclaredType->Clone();
        AddedVariable->Name = ExpectIdentifier( TokenPosition );
        
        // special error report for standard C arrays
        if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBracket ) )
          RaiseFatalError( (*TokenPosition)->Location, "in this compiler, arrays need to be declared as 'type[n] name', not 'type name[n]'" );
        
        // optionally, an assignment can be made on declaration
        if( TokenIsThisOperator( *TokenPosition, OperatorTypes::Equal ) )
        {
            // consume equal sign
            TokenPosition++;
            
            // case 1: initialization list
            if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBrace ) )
              AddedVariable->InitialValue = ParseInitializationList( AddedVariable, TokenPosition );
            
            // case 2: expression
            else
              AddedVariable->InitialValue = ParseExpression( AddedVariable, TokenPosition );
        }
        
        // additional variable has been parsed
        AddedVariable->AllocateAsVariable();
        VariableList->Variables.push_back( AddedVariable );
    }
    
    // in any case, expect a semicolon as ending
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    return VariableList;
}

// -----------------------------------------------------------------------------

InitializationListNode* VirconCParser::ParseInitializationList( CNode* Parent, CTokenIterator& TokenPosition )
{
    // consume open brace
    TokenPosition++;
    
    // create new initialization list
    InitializationListNode* InitializationList = new InitializationListNode( Parent );
    InitializationList->Location = (*TokenPosition)->Location;
    
    // keep adding values until closing brace is found
    while( !IsLastToken( *TokenPosition ) )
    {
        CToken* NextToken = *TokenPosition;
        
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ) )
        {
            TokenPosition++;
            break;
        }
        
        // if this is not the first value,
        // expect a comma as separation
        if( !InitializationList->AssignedValues.empty() )
          ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Comma );
        
        // CASE 1: next value is a nested initialization list
        if( TokenIsThisDelimiter( *TokenPosition, DelimiterTypes::OpenBrace ) )
          InitializationList->AssignedValues.push_back( ParseInitializationList( InitializationList, TokenPosition ) );
        
        // CASE 2: next value is an expression
        else
          InitializationList->AssignedValues.push_back( ParseExpression( InitializationList, TokenPosition ) );
    }
    
    return InitializationList;
}

// -----------------------------------------------------------------------------

// does NOT expect type and name to be consumed
// (because, in a union, a member is actually expected)
MemberNode* VirconCParser::ParseMember( UnionNode* OwnerUnion, CTokenIterator& TokenPosition )
{
    MemberNode* NewMember = new MemberNode( OwnerUnion );
    NewMember->Location = (*TokenPosition)->Location;
    
    // parse the type first
    NewMember->DeclaredType = ParseType( OwnerUnion, TokenPosition );
    
    // expect an identifier as name
    NewMember->Name = ExpectIdentifier( TokenPosition );
    
    // for members there can be no initial assignment!
    // here, allow only 1 member to be declared at once
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // allocate this member in the union
    NewMember->AllocateInGroup();
    return NewMember;
}

// -----------------------------------------------------------------------------

// does NOT expect type and first name to be consumed
// (because, in a structure, a member list is actually expected)
MemberListNode* VirconCParser::ParseMemberList( StructureNode* OwnerStructure, CTokenIterator& TokenPosition )
{
    MemberListNode* MemberList = new MemberListNode( OwnerStructure );
    MemberList->Location = (*TokenPosition)->Location;
    
    // parse the type first
    MemberList->DeclaredType = ParseType( OwnerStructure, TokenPosition );
    
    // create a first member
    MemberNode* NewMember = new MemberNode( MemberList );
    NewMember->Location = (*TokenPosition)->Location;
    NewMember->DeclaredType = MemberList->DeclaredType->Clone();
    
    // expect an identifier as name
    NewMember->Name = ExpectIdentifier( TokenPosition );
    
    // for members there can be no initial assignment!
    
    // initial member has been parsed
    NewMember->AllocateInGroup();
    OwnerStructure->MembersInOrder.push_back( NewMember );
    MemberList->Members.push_back( NewMember );
    
    // there can be multiple declarations of the
    // same type together, separated by commas
    while( !IsLastToken( *TokenPosition ) )
    {
        if( !TokenIsThisSymbol( *TokenPosition, SpecialSymbolTypes::Comma ) )
          break;
        
        // consume the comma
        TokenPosition++;
        
        // parse another member
        MemberNode* AddedMember = new MemberNode( MemberList );
        AddedMember->Location = (*TokenPosition)->Location;
        AddedMember->DeclaredType = MemberList->DeclaredType->Clone();
        AddedMember->Name = ExpectIdentifier( TokenPosition );
        
        // for members there can be no initial assignment!
        
        // additional member has been parsed
        AddedMember->AllocateInGroup();
        OwnerStructure->MembersInOrder.push_back( AddedMember );
        MemberList->Members.push_back( AddedMember );
    }
    
    // in any case, expect a semicolon as ending
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    return MemberList;
}

// -----------------------------------------------------------------------------

StructureNode* VirconCParser::ParseStructure( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create new node and consume "struct" keyword
    StructureNode* NewStructure = new StructureNode( Parent );
    NewStructure->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // read structure name
    NewStructure->Name = ExpectIdentifier( TokenPosition );
    
    // define this structure in the scope early, so that
    // it can self-reference itself through pointers
    NewStructure->DeclaredType = new StructureType( NewStructure );
    NewStructure->AllocateName();
    
    // expect an open brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing member lists until we find a closing brace
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( NewStructure->Location, "structure is not closed (unexpected end of file)" );
        
        // CASE 2: end of enumeration
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: member list
        MemberListNode* NewMemberList = ParseMemberList( NewStructure, TokenPosition );
        NewStructure->Declarations.push_back( NewMemberList );
    }
    
    // as ending, expect  a semicolon
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // ensure that the structure has at least 1 member
    if( NewStructure->Declarations.empty() )
      RaiseError( NewStructure->Location, "structures must have at least 1 member" );
    
    return NewStructure;
}

// -----------------------------------------------------------------------------

UnionNode* VirconCParser::ParseUnion( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create new node and consume "union" keyword
    UnionNode* NewUnion = new UnionNode( Parent );
    NewUnion->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // read union name
    NewUnion->Name = ExpectIdentifier( TokenPosition );
    
    // define this union in the scope early, so that
    // it can self-reference itself through pointers
    NewUnion->DeclaredType = new UnionType( NewUnion );
    NewUnion->AllocateName();
    
    // expect an open brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing individual members until we find a closing brace
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( NewUnion->Location, "union is not closed (unexpected end of file)" );
        
        // CASE 2: end of enumeration
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: union member
        MemberNode* NewMember = ParseMember( NewUnion, TokenPosition );
        NewUnion->Declarations.push_back( NewMember );
    }
    
    // as ending, expect  a semicolon
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // ensure that the union has at least 1 member
    if( NewUnion->Declarations.empty() )
      RaiseError( NewUnion->Location, "unions must have at least 1 member" );
    
    return NewUnion;
}

// -----------------------------------------------------------------------------

EnumValueNode* VirconCParser::ParseEnumValue( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create new node
    EnumValueNode* NewEnumValue = new EnumValueNode( Parent );
    NewEnumValue->Location = (*TokenPosition)->Location;
    
    // expect an identifier with the name
    NewEnumValue->Name = ExpectIdentifier( TokenPosition );
    
    // assignment is optional, so if next token
    // is an equal sign parse an expression next
    if( TokenIsThisOperator( *TokenPosition, OperatorTypes::Equal ) )
    {
        TokenPosition++;
        NewEnumValue->ValueExpression = ParseExpression( NewEnumValue, TokenPosition );
    }
    
    // define this value in the enumeration
    NewEnumValue->AllocateInEnum();
    return NewEnumValue;
}

// -----------------------------------------------------------------------------

EnumerationNode* VirconCParser::ParseEnumeration( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create new node and consume "enum" keyword
    EnumerationNode* NewEnumeration = new EnumerationNode( Parent );
    NewEnumeration->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // read enumeration name
    NewEnumeration->Name = ExpectIdentifier( TokenPosition );
    
    // expect an open brace
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenBrace );
    
    // keep parsing enumeration values until we find a closing brace
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // CASE 1: end of file
        if( IsLastToken(NextToken) )
          RaiseFatalError( NewEnumeration->Location, "enumeration is not closed (unexpected end of file)" );
        
        // CASE 2: end of enumeration
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseBrace ))
        {
            // consume the brace and exit
            TokenPosition++;
            break;
        }
        
        // CASE 3: for an enumeration value, if it is not
        // the first one, expect a comma as separation
        if( !NewEnumeration->Values.empty() )
          ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Comma );
        
        // now actually parse the enumeration value
        ParseEnumValue( NewEnumeration, TokenPosition );
    }
    
    // as ending, expect a semicolon
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // (enumerations may actually be empty, so don't check that)
    
    // create the structure type
    NewEnumeration->DeclaredType = new EnumerationType( NewEnumeration );
    
    // define this enumeration in the scope
    NewEnumeration->AllocateName();
    return NewEnumeration;
}

// -----------------------------------------------------------------------------

TypedefNode* VirconCParser::ParseTypedef( CNode* Parent, CTokenIterator& TokenPosition )
{
    // consume typedef keyword
    TokenPosition++;
    
    // create new typedef
    TypedefNode* Typedef = new TypedefNode( Parent );
    Typedef->Location = (*TokenPosition)->Location;
    
    // parse the declared type
    Typedef->DeclaredType = ParseType( Typedef, TokenPosition );
    
    // expect the defined name
    Typedef->Name = ExpectIdentifier( TokenPosition );
    
    // expect a semicolon as ending
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    
    // allocate the new type in owner scope
    Typedef->AllocateName();
    return Typedef;
}

// -----------------------------------------------------------------------------

EmbeddedFileNode* VirconCParser::ParseEmbeddedFile( CNode* Parent, CTokenIterator& TokenPosition )
{
    // parent node can only be the top level
    if( Parent->Type() != CNodeTypes::TopLevel )
      RaiseFatalError( (*TokenPosition)->Location, "embedded files can only be declared at the top level" );
    
    // consume embedded keyword
    TokenPosition++;
    
    // create new embedded file
    EmbeddedFileNode* EmbeddedFile = new EmbeddedFileNode( Parent );
    EmbeddedFile->Location = (*TokenPosition)->Location;
    
    // We will parse the declared variable manually, otherwise
    // we would incorrectly parse the file path. It is also
    // needed to avoid allocating the variable as usual
    
    // parse the variable type
    DataType* VariableType = ParseType( EmbeddedFile, TokenPosition );
    
    // expect the variable name
    string VariableName = ExpectIdentifier( TokenPosition );
    
    // create the new variable
    EmbeddedFile->Variable = new VariableNode( EmbeddedFile );
    EmbeddedFile->Variable->Location = (*TokenPosition)->Location;
    EmbeddedFile->Variable->DeclaredType = VariableType;
    EmbeddedFile->Variable->Name = VariableName;
    
    // allocate variable name in top level scope
    ((TopLevelNode*)Parent)->DeclareNewIdentifier( VariableName, EmbeddedFile->Variable );
    
    // allocate variable as a global, but using the
    // cartridge program rom instead of the main ram
    EmbeddedFile->Variable->Placement.IsEmbedded = true;
    EmbeddedFile->Variable->Placement.EmbeddedName = VariableName;
    
    // expect an equal symbol before the file path string
    ExpectOperator( TokenPosition, OperatorTypes::Equal );
    
    // read the file path string
    if( (*TokenPosition)->Type() != CTokenTypes::LiteralString )
      RaiseFatalError( (*TokenPosition)->Location, "expected file path string" );
    
    LiteralStringToken* PathToken = (LiteralStringToken*)(*TokenPosition);
    EmbeddedFile->FilePath = PathToken->Value;
    TokenPosition++;
    
    // expect a semicolon as ending
    ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Semicolon );
    return EmbeddedFile;
}


// =============================================================================
//      VIRCON C PARSER: PARSERS FOR EXPRESSIONS
// =============================================================================


ExpressionNode* VirconCParser::ParseExpression( CNode* Parent, CTokenIterator& TokenPosition, bool Greedy )
{
    CToken* NextToken = *TokenPosition;
    ExpressionNode* PrimaryExpression = nullptr;
    
    // first discard all invalid cases
    if( IsLastToken(NextToken) )
      RaiseFatalError( NextToken->Location, "unexpected end of file (expected an expression)" );
    
    if( !IsValidStartOfExpression( NextToken ) )
      RaiseFatalError( NextToken->Location, "invalid start of expression: " + NextToken->ToString() );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 1: FORM THE BASE EXPRESSION
    // just choose from all valid cases
    // (careful! any of these cases may later have to
    // be converted into a binary operation expression)
    
    // CASE 1: SizeOf expressions
    if( TokenIsThisKeyword( NextToken, KeywordTypes::SizeOf ) )
      PrimaryExpression = ParseSizeOf( Parent, TokenPosition );
      
    // CASE 2: Enclosed expressions
    if( TokenIsThisDelimiter( NextToken, DelimiterTypes::OpenParenthesis ) )
      PrimaryExpression = ParseEnclosedExpression( Parent, TokenPosition );
    
    // CASE 3: String literals
    if( NextToken->Type() == CTokenTypes::LiteralString )
      PrimaryExpression = ParseLiteralString( Parent, TokenPosition );
    
    // CASE 4: Unary operator
    else if( TokenIsUnaryOperator( NextToken ) )
      PrimaryExpression = ParseUnaryOperation( Parent, TokenPosition );
    
    // CASE 5: Function call
    else if( NextToken->Type() == CTokenTypes::Identifier )
    {
        CToken* FollowingToken = *Next( TokenPosition );
        
        if( TokenIsThisDelimiter( FollowingToken, DelimiterTypes::OpenParenthesis ) )
          PrimaryExpression = ParseFunctionCall( Parent, TokenPosition );
    }
    
    // CASE 6: Atom (all other cases)
    if( !PrimaryExpression )
      PrimaryExpression = ParseExpressionAtom( Parent, TokenPosition );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 2: EXPAND BASE EXPRESSION WITH NON-BINARY CONNECTORS
    
    while( !IsLastToken( *TokenPosition ))
    {
        NextToken = *TokenPosition;
        
        // CASE 1: Array access
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::OpenBracket ) )
          PrimaryExpression = ParseArrayAccess( Parent, PrimaryExpression, TokenPosition );
        
        // CASE 2: Member access
        else if( TokenIsThisOperator( NextToken, OperatorTypes::Dot ))
          PrimaryExpression = ParseMemberAccess( Parent, PrimaryExpression, TokenPosition );
        
        // CASE 3: Pointed member access
        else if( TokenIsThisOperator( NextToken, OperatorTypes::MinusGreater ))
          PrimaryExpression = ParsePointedMemberAccess( Parent, PrimaryExpression, TokenPosition );
          
        // CASE 4: Post increment
        else if( TokenIsThisOperator( NextToken, OperatorTypes::DoublePlus ))
        {
            // consume the operator
            TokenPosition++;
            
            // create new node
            UnaryOperationNode* UnaryOperation = new UnaryOperationNode( Parent );
            UnaryOperation->Location = (*TokenPosition)->Location;
            UnaryOperation->Operand = PrimaryExpression;
            UnaryOperation->Operator = UnaryOperators::PostIncrement;
            
            // replace primary expression
            PrimaryExpression->Parent = UnaryOperation;
            PrimaryExpression = UnaryOperation;
        }
        
        // CASE 5: Post decrement
        else if( TokenIsThisOperator( NextToken, OperatorTypes::DoubleMinus ))
        {
            // consume the operator
            TokenPosition++;
            
            // create new node
            UnaryOperationNode* UnaryOperation = new UnaryOperationNode( Parent );
            UnaryOperation->Location = (*TokenPosition)->Location;
            UnaryOperation->Operand = PrimaryExpression;
            UnaryOperation->Operator = UnaryOperators::PostDecrement;
            
            // replace primary expression
            PrimaryExpression->Parent = UnaryOperation;
            PrimaryExpression = UnaryOperation;
        }
        
        else break;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 3: EXPAND BASE EXPRESSION WITH BINARY OPERATIONS
    // Expand the expression into a binary operation tree
    // if an operator is found after the first term
    
    if( Greedy )
    {
        while( !IsLastToken( *TokenPosition ))
        {
            NextToken = *TokenPosition;
            
            // CASE 1: No binary operator found
            if( !TokenIsBinaryOperator( NextToken ) )
              break;
            
            OperatorToken* AddedOperator = (OperatorToken*)NextToken;
            TokenPosition++;
            
            // CASE 2: form the first binary operation
            if( PrimaryExpression->Type() != CNodeTypes::BinaryOperation )
            {
                PrimaryExpression = ParseBinaryOperation( Parent, PrimaryExpression, AddedOperator, TokenPosition );
                continue;
            }
            
            // CASE 3: expand the tree attending to operator precedences
            ExpressionNode* AddedOperand = ParseExpression( Parent, TokenPosition, false );
            PrimaryExpression = AddBinaryOperationToTree( (BinaryOperationNode*)PrimaryExpression, AddedOperator, AddedOperand );
        }
    }
    
    // we need to allocate this block's local
    // variables in its containing function
    PrimaryExpression->AllocateTemporaries();
    
    // otherwise the primary expression
    // parsed earlier is autonomous
    return PrimaryExpression;
}

// -----------------------------------------------------------------------------

ExpressionAtomNode* VirconCParser::ParseExpressionAtom( CNode* Parent, CTokenIterator& TokenPosition )
{
    // an atom is always formed by only 1 token
    CToken* AtomToken = *TokenPosition;
    TokenPosition++;
    
    // create the atom
    ExpressionAtomNode* Atom = new ExpressionAtomNode( Parent );
    Atom->Location = AtomToken->Location;
    
    // CASE 1: Literal values
    if( AtomToken->Type() == CTokenTypes::LiteralValue )
    {
        LiteralValueToken* Literal = (LiteralValueToken*)AtomToken;
        
        // 1-A: Literal integer
        if( Literal->ValueType == LiteralValueTypes::Int )
        {
            Atom->AtomType = AtomTypes::LiteralInteger;
            Atom->IntValue = Literal->IntValue;
            return Atom;
        }
        
        // 1-B: Literal float
        if( Literal->ValueType == LiteralValueTypes::Float )
        {
            Atom->AtomType = AtomTypes::LiteralFloat;
            Atom->FloatValue = Literal->FloatValue;
            return Atom;
        }
        // 1-C: Literal boolean
        else
        {
            Atom->AtomType = AtomTypes::LiteralBoolean;
            Atom->BoolValue = Literal->BoolValue;
            return Atom;
        }
    }
    
    // CASE 2: Identifier (variable access/enum value)
    else if( AtomToken->Type() == CTokenTypes::Identifier )
    {
        IdentifierToken* Identifier = (IdentifierToken*)AtomToken;
        Atom->IdentifierName = Identifier->Name;
        
        // before returning it, resolve the identifier
        Atom->ResolveIdentifier();
        return Atom;
    }
    
    // CASE 3: NULL keyword
    else if( AtomToken->Type() == CTokenTypes::Keyword )
    {
        // parse NULL as integer -1
        if( TokenIsThisKeyword( AtomToken, KeywordTypes::Null ))
        {
            Atom->AtomType = AtomTypes::LiteralInteger;
            Atom->IntValue = -1;
            return Atom;
        }
    }
    
    // should never be reached, but still
    RaiseFatalError( AtomToken->Location, "invalid expression atom" );
    return nullptr;
}

// -----------------------------------------------------------------------------

FunctionCallNode* VirconCParser::ParseFunctionCall( CNode* Parent, CTokenIterator& TokenPosition )
{
    FunctionCallNode* FunctionCall = new FunctionCallNode( Parent );
    FunctionCall->Location = (*TokenPosition)->Location;
    
    // here we already know we will find name + parenthesis
    FunctionCall->FunctionName = ExpectIdentifier( TokenPosition );
    TokenPosition++;
    
    while( !IsLastToken( *TokenPosition ) )
    {
        CToken* NextToken = *TokenPosition;
        
        // keep adding parameters until
        // we find a closing parenthesis
        if( TokenIsThisDelimiter( NextToken, DelimiterTypes::CloseParenthesis ) )
        {
            TokenPosition++;
            break;
        }
        
        // if this is not the first parameter,
        // expect a comma as separation
        if( !FunctionCall->Parameters.empty() )
          ExpectSpecialSymbol( TokenPosition, SpecialSymbolTypes::Comma );
        
        // expect an expression as next parameter
        FunctionCall->Parameters.push_back( ParseExpression( Parent, TokenPosition ) );
    }
    
    // before returning it, resolve the function
    FunctionCall->ResolveFunction();
    
    // allocate the function call in stack
    FunctionCall->AllocateCallSpace();
    
    return FunctionCall;
}

// -----------------------------------------------------------------------------

ArrayAccessNode* VirconCParser::ParseArrayAccess( CNode* Parent, ExpressionNode* ArrayOperand, CTokenIterator& TokenPosition )
{
    // create new node
    ArrayAccessNode* ArrayAccess = new ArrayAccessNode( Parent );
    ArrayAccess->Location = (*TokenPosition)->Location;
    ArrayAccess->ArrayOperand = ArrayOperand;
    
    // correct array operand's parent
    ArrayAccess->ArrayOperand->Parent = ArrayAccess;
    
    // consume the open bracket
    TokenPosition++;
    
    // parse the index expression
    ArrayAccess->IndexOperand = ParseExpression( ArrayAccess, TokenPosition, true );
    
    // expect a closing bracket
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseBracket );
    return ArrayAccess;
}

// -----------------------------------------------------------------------------

UnaryOperationNode* VirconCParser::ParseUnaryOperation( CNode* Parent, CTokenIterator& TokenPosition )
{
    UnaryOperationNode* Operation = new UnaryOperationNode( Parent );
    
    // here we already know next token is a unary operator
    CToken* NextToken = *TokenPosition;
    Operation->Operator = TokenToUnaryOperator( (OperatorToken*)NextToken );
    Operation->Location = NextToken->Location;
    TokenPosition++;
    
    // now just parse the operand expression
    // (but retain unary operation as an independent expression)
    Operation->Operand = ParseExpression( Operation, TokenPosition, false );
    return Operation;
}

// -----------------------------------------------------------------------------

BinaryOperationNode* VirconCParser::ParseBinaryOperation( CNode* Parent, ExpressionNode* LeftOperand, OperatorToken* Operator, CTokenIterator& TokenPosition )
{
    BinaryOperationNode* Operation = new BinaryOperationNode( Parent );
    Operation->Location = (*TokenPosition)->Location;
    Operation->LeftOperand = LeftOperand;
    Operation->Operator = TokenToBinaryOperator( Operator );
    
    // correct left operand's parent
    Operation->LeftOperand->Parent = Operation;
    
    // now just parse the right expression
    Operation->RightOperand = ParseExpression( Operation, TokenPosition, false );
    return Operation;
}

// -----------------------------------------------------------------------------

EnclosedExpressionNode* VirconCParser::ParseEnclosedExpression( CNode* Parent, CTokenIterator& TokenPosition )
{
    // consume the open parenthesis
    EnclosedExpressionNode* Enclosed = new EnclosedExpressionNode( Parent );
    Enclosed->Location = (*TokenPosition)->Location;
    TokenPosition++;
    
    // parse the internal expression
    Enclosed->InternalExpression = ParseExpression( Enclosed, TokenPosition );
    
    // expect a closing parenthesis
    CToken* EndToken = *TokenPosition;
    TokenPosition++;
    
    if( TokenIsThisDelimiter( EndToken, DelimiterTypes::CloseParenthesis ) )
      return Enclosed;
    
    RaiseError( Enclosed->Location, "at this open parenthesis:" );
    RaiseFatalError( EndToken->Location, "expected a closing parenthesis" );
}

// -----------------------------------------------------------------------------

MemberAccessNode* VirconCParser::ParseMemberAccess( CNode* Parent, ExpressionNode* LeftOperand, CTokenIterator& TokenPosition )
{
    // consume dot operator
    TokenPosition++;
    
    // create new node
    MemberAccessNode* MemberAccess = new MemberAccessNode( Parent );
    MemberAccess->Location = (*TokenPosition)->Location;
    MemberAccess->GroupOperand = LeftOperand;
    
    // correct group operand's parent
    MemberAccess->GroupOperand->Parent = MemberAccess;
    
    // parse the right operand as a member reference
    MemberAccess->MemberName = ExpectIdentifier( TokenPosition );
    
    // before returning it, resolve the member
    MemberAccess->ResolveMember();
    return MemberAccess;
}

// -----------------------------------------------------------------------------

PointedMemberAccessNode* VirconCParser::ParsePointedMemberAccess( CNode* Parent, ExpressionNode* LeftOperand, CTokenIterator& TokenPosition )
{
    // consume arrow operator
    TokenPosition++;
    
    // create new node
    PointedMemberAccessNode* PointedMemberAccess = new PointedMemberAccessNode( Parent );
    PointedMemberAccess->Location = (*TokenPosition)->Location;
    PointedMemberAccess->GroupOperand = LeftOperand;
    
    // correct group operand's parent
    PointedMemberAccess->GroupOperand->Parent = PointedMemberAccess;
    
    // parse the right operand as a member reference
    PointedMemberAccess->MemberName = ExpectIdentifier( TokenPosition );
    
    // before returning it, resolve the member
    PointedMemberAccess->ResolveMember();
    return PointedMemberAccess;
}

// -----------------------------------------------------------------------------

SizeOfNode* VirconCParser::ParseSizeOf( CNode* Parent, CTokenIterator& TokenPosition )
{
    // consume sizeof keyword
    TokenPosition++;
    
    // create new node
    SizeOfNode* SizeOf = new SizeOfNode( Parent );
    SizeOf->Location = (*TokenPosition)->Location;
    
    // expect an open parenthesis
    ExpectDelimiter( TokenPosition, DelimiterTypes::OpenParenthesis );
    
    // CASE 1: size for a type
    if( IsValidStartOfType( *TokenPosition, Parent ) )
      SizeOf->QueriedType = ParseType( Parent, TokenPosition );
    
    // CASE 2: size for an expression
    else
      SizeOf->QueriedExpression = ParseExpression( SizeOf, TokenPosition, true );
        
    // in any case, expect a closing parenthesis as end
    ExpectDelimiter( TokenPosition, DelimiterTypes::CloseParenthesis );
    return SizeOf;
}

// -----------------------------------------------------------------------------

LiteralStringNode* VirconCParser::ParseLiteralString( CNode* Parent, CTokenIterator& TokenPosition )
{
    // create new node
    LiteralStringNode* String = new LiteralStringNode( Parent );
    String->Location = (*TokenPosition)->Location;
    
    // keep adding content by concatenating
    // all consecutive strings as in regular C
    while( (*TokenPosition)->Type() == CTokenTypes::LiteralString )
    {
        LiteralStringToken* StringToken = (LiteralStringToken*)(*TokenPosition);
        String->Value += StringToken->Value;
        TokenPosition++;
    }
    
    return String;
}


// =============================================================================
//      SPECIFICS FOR BINARY OPERATIONS
// =============================================================================


BinaryOperationNode* VirconCParser::AddBinaryOperationToTree( BinaryOperationNode* PreviousTree, OperatorToken* AddedOperator, ExpressionNode* AddedOperand )
{
    // preparations common for all cases
    BinaryOperationNode* AddedOperation = new BinaryOperationNode( nullptr );
    AddedOperation->Location = AddedOperator->Location;
    AddedOperation->Operator = TokenToBinaryOperator( AddedOperator );
    AddedOperation->RightOperand = AddedOperand;
    
    // CASE 1: Lowest priority in the tree
    if( !NewBinaryOperatorHasPrecedence( PreviousTree->Operator, AddedOperation->Operator ) )
    {
        // use the whole previous tree as left operand
        AddedOperation->LeftOperand = PreviousTree;
        AddedOperation->Parent = PreviousTree->Parent;
        PreviousTree->Parent = AddedOperation;
        return AddedOperation;
    }
    
    // We will process the rightwards ladder down the tree
    // searching for the point where the new operator precedence
    // fits to maintain the order (deeper = higher precedence)
    BinaryOperationNode* CurrentOperation = PreviousTree;
    BinaryOperationNode* ParentOperation = nullptr;
    
    while( true )
    {
        // CASE 2: Priority is in-between tree levels
        if( !NewBinaryOperatorHasPrecedence( CurrentOperation->Operator, AddedOperation->Operator ) )
        {
            AddedOperation->LeftOperand = CurrentOperation;
            CurrentOperation->Parent = AddedOperation;
            ParentOperation->RightOperand = AddedOperation;
            AddedOperation->Parent = ParentOperation;
            return PreviousTree;
        }
        
        // otherwise check if we can go deeper
        if( CurrentOperation->RightOperand->Type() != CNodeTypes::BinaryOperation )
          break;
        
        // otherwise go deeper
        ParentOperation = CurrentOperation;
        CurrentOperation = (BinaryOperationNode*)(CurrentOperation->RightOperand);
    }
    
    // CASE 3: Highest priority in the tree
    // replace only the rightmost leaf with the new operation
    AddedOperation->LeftOperand = CurrentOperation->RightOperand;
    AddedOperation->Parent = CurrentOperation;
    CurrentOperation->RightOperand = AddedOperation;
    return PreviousTree;
}


// =============================================================================
//      VIRCON C PARSER: MAIN PARSER
// =============================================================================


void VirconCParser::ParseTopLevel( CTokenList& Tokens_ )
{
    // capture the new token list
    Tokens = &Tokens_;
    
    // create a program AST
    delete ProgramAST;
    ProgramAST = new TopLevelNode;
    
    // parse the whole token list
    CTokenIterator TokenPosition = Tokens->begin();
    ProgramAST->Location = (*TokenPosition)->Location;
    
    while( TokenPosition != Tokens->end() )
    {
        CToken* NextToken = *TokenPosition;
        
        // Start and end tokens are ignored here.
        // They are only used by specialized parsers
        if( IsFirstToken(NextToken) )
        {
            TokenPosition++;
            continue;
        }
        
        if( IsLastToken(NextToken) )
          break;
        
        // recognize function & variable declarations
        if( IsValidStartOfType( NextToken, ProgramAST ) )
        {
            CNode* NewDeclaration = ParseDeclaration( ProgramAST, TokenPosition, true );
            ProgramAST->Statements.push_back( NewDeclaration );
            continue;
        }
        
        // recognize type declarations
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Struct ))
        {
            StructureNode* NewStructure = ParseStructure( ProgramAST, TokenPosition );
            ProgramAST->Statements.push_back( NewStructure );
            continue;
        }
        
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Union ))
        {
            UnionNode* NewUnion = ParseUnion( ProgramAST, TokenPosition );
            ProgramAST->Statements.push_back( NewUnion );
            continue;
        }
        
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Enum ))
        {
            EnumerationNode* NewEnumeration = ParseEnumeration( ProgramAST, TokenPosition );
            ProgramAST->Statements.push_back( NewEnumeration );
            continue;
        }
        
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Typedef ) )
        {
            TypedefNode* NewTypedef = ParseTypedef( ProgramAST, TokenPosition );
            ProgramAST->Statements.push_back( NewTypedef );
            continue;
        }
        
        // recognize embedded files
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Embedded ) )
        {
            EmbeddedFileNode* NewEmbeddedFile = ParseEmbeddedFile( ProgramAST, TokenPosition );
            ProgramAST->Statements.push_back( NewEmbeddedFile );
            continue;
        }
        
        // give special error messages for non supported features of standard C
        if( NextToken->Type() == CTokenTypes::Identifier )
        {
            IdentifierToken* IT = (IdentifierToken*)NextToken;
            
            if( IT->Name == "const" )
              RaiseFatalError( NextToken->Location, "const variables are not supported in this compiler" );
            
            if( IT->Name == "enum" )
              RaiseFatalError( NextToken->Location, "enumerations are not supported in this compiler" );
        }
        
        // any other cases are not valid
        // (but choose a message depending on the reason)
        if( !IsValidStartOfStatement( NextToken ) )
          RaiseFatalError( NextToken->Location, "invalid start of statement" );
        
        else
          RaiseFatalError( NextToken->Location, "start of a statement not valid outside a function: " + NextToken->ToString() );
    }
}
