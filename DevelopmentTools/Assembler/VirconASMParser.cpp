// *****************************************************************************
    // include project headers
    #include "VirconASMParser.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      VIRCON ASM PARSER: INSTANCE HANDLING
// =============================================================================


VirconASMParser::~VirconASMParser()
{
    for( ASTNode* Node : ProgramAST )
      delete Node;
      
    for( Token* T : Tokens )
      delete T;
}


// =============================================================================
//      VIRCON ASM PARSER: ERROR HANDLING
// =============================================================================


void VirconASMParser::EmitError( SourceLocation Location, const string& Description, bool Abort )
{
    cerr << Location.FilePath << ':' << Location.Line;
    cerr << ": parser error: " << Description << endl;
    
    if( Abort )
      throw runtime_error( "assembly terminated" );
}

// -----------------------------------------------------------------------------

void VirconASMParser::EmitWarning( SourceLocation Location, const string& Description )
{
    cerr << Location.FilePath << ':' << Location.Line;
    cerr << ": parser warning: " << Description << endl;
}


// =============================================================================
//      VIRCON ASM PARSER: MISCELLANEOUS FUNCTIONS
// =============================================================================


void VirconASMParser::ExpectSameLine( Token* Start, Token* Current )
{
    if( !AreInSameLine( Start, Current ) )
      EmitError( Start->Location, "unexpected end of line" );
}

// -----------------------------------------------------------------------------

void VirconASMParser::ExpectEndOfLine( Token* Start, Token* Current )
{
    if( AreInSameLine( Start, Current ) )
      EmitError( Start->Location, "expected end of line" );
}


// =============================================================================
//      VIRCON ASM PARSER: CONTEXTUAL PARSING FUNCTIONS
// =============================================================================


BasicValue VirconASMParser::ParseBasicValue( TokenIterator& TokenPosition )
{
    BasicValue Value;
    Token* StartToken = *TokenPosition;
    TokenPosition++;
    
    // CASE 1: signed numbers
    if( TokenIsThisSymbol( StartToken, SymbolTypes::Plus ) || TokenIsThisSymbol( StartToken, SymbolTypes::Minus ) )
    {
        Token* NumberToken = *TokenPosition;
        TokenPosition++;
        
        ExpectSameLine( StartToken, NumberToken );
        
        if( NumberToken->Type() == TokenTypes::LiteralInteger )
        {
            Value.Type = BasicValueTypes::LiteralInteger;
            Value.IntegerField = ((LiteralIntegerToken*)NumberToken)->Value;
            
            if( TokenIsThisSymbol( StartToken, SymbolTypes::Minus ) )
              Value.IntegerField = -Value.IntegerField;
            
            return Value;
        }
        
        else if( NumberToken->Type() == TokenTypes::LiteralFloat )
        {
            Value.Type = BasicValueTypes::LiteralFloat;
            Value.FloatField = ((LiteralFloatToken*)NumberToken)->Value;
            
            if( TokenIsThisSymbol( StartToken, SymbolTypes::Minus ) )
              Value.FloatField = -Value.FloatField;
            
            return Value;
        }
        
        else EmitError( StartToken->Location, "expected number literal after sign" );
    }
    
    // CASE 2: Unsigned integers
    else if( StartToken->Type() == TokenTypes::LiteralInteger )
    {
        Value.Type = BasicValueTypes::LiteralInteger;
        Value.IntegerField = ((LiteralIntegerToken*)StartToken)->Value;
        return Value;
    }
    
    // CASE 3: Unsigned floats
    else if( StartToken->Type() == TokenTypes::LiteralFloat )
    {
        Value.Type = BasicValueTypes::LiteralFloat;
        Value.FloatField = ((LiteralFloatToken*)StartToken)->Value;
        return Value;
    }
    
    // CASE 4: Labels
    else if( StartToken->Type() == TokenTypes::Label )
    {
        Value.Type = BasicValueTypes::Label;
        Value.LabelField = ((LabelToken*)StartToken)->Name;
        return Value;
    }
    
    // CASE 5: HW Registers
    else if( StartToken->Type() == TokenTypes::CPURegister )
    {
        Value.Type = BasicValueTypes::CPURegister;
        Value.RegisterField = ((CPURegisterToken*)StartToken)->Which;
        return Value;
    }
    
    // CASE 6: I/O Ports
    else if( StartToken->Type() == TokenTypes::IOPort )
    {
        Value.Type = BasicValueTypes::IOPort;
        Value.PortField = ((IOPortToken*)StartToken)->Which;
        return Value;
    }
    
    // CASE 7: I/O Port values
    else if( StartToken->Type() == TokenTypes::IOPortValue )
    {
        Value.Type = BasicValueTypes::IOPortValue;
        Value.PortValueField = ((IOPortValueToken*)StartToken)->Which;
        return Value;
    }
    
    // OTHERS
    EmitError( StartToken->Location, "expected basic value" );
    
    // this prevents a compiler warning
    return Value;
}

// -----------------------------------------------------------------------------

InstructionOperand VirconASMParser::ParseOperand( TokenIterator& TokenPosition )
{
    InstructionOperand Operand;
    Token* StartToken = *TokenPosition;
    
    // first check for open bracket
    if( TokenIsThisSymbol( StartToken, SymbolTypes::OpenBracket ) )
    {
        Operand.IsMemoryAddress = true;
        
        // base should stay in the same line
        TokenPosition++;
        ExpectSameLine( StartToken, *TokenPosition );
    }
    
    // read operand base
    Operand.Base = ParseBasicValue( TokenPosition );
    
    // extra parsing after a base offset
    if( Operand.IsMemoryAddress )
    {
        // detect possible presence of an offset
        Token* AfterBaseToken = *TokenPosition;
        
        if( !AreInSameLine( StartToken, AfterBaseToken ) )
          EmitError( StartToken->Location, "expected closing bracket" );
        
        if( TokenIsThisSymbol( AfterBaseToken, SymbolTypes::Plus ) || TokenIsThisSymbol( AfterBaseToken, SymbolTypes::Minus ) )
        {
            Operand.HasOffset = true;
            Operand.Offset = ParseBasicValue( TokenPosition );
        }
        
        else if( !TokenIsThisSymbol( AfterBaseToken, SymbolTypes::CloseBracket ) )
          EmitError( StartToken->Location, "incorrect memory address" );
        
        // expect a closing bracket in the same line
        Token* ClosingToken = *TokenPosition;
        
        if( !TokenIsThisSymbol( (*TokenPosition), SymbolTypes::CloseBracket )
            || !AreInSameLine( StartToken, ClosingToken )
          )
          EmitError( StartToken->Location, "expected closing bracket" );
        
        TokenPosition++;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // operand has been parsed, now validate it
    
    // addresses with offsets can only be [register + integer]
    if( Operand.HasOffset )
    {
        if( Operand.Base.Type != BasicValueTypes::CPURegister
        ||  Operand.Offset.Type != BasicValueTypes::LiteralInteger )
          EmitError( StartToken->Location, "memory addresses with offset must be in the form [register +/- integer]" );
    }
    
    // addresses without offsets must be either register, integer or label
    if( Operand.IsMemoryAddress && !Operand.HasOffset )
    {
        if( Operand.Base.Type != BasicValueTypes::CPURegister
        &&  Operand.Base.Type != BasicValueTypes::LiteralInteger
        &&  Operand.Base.Type != BasicValueTypes::Label )
          EmitError( StartToken->Location, "invalid memory address (must be register, integer or label)" );
    }
    
    // interpret an offset of zero as no offset
    if( Operand.HasOffset )
      if( Operand.Offset.IntegerField == 0 )
        Operand.HasOffset = false;
    
    // now, we can procide the operand safely
    return Operand;
}

// -----------------------------------------------------------------------------

InstructionNode* VirconASMParser::ParseInstruction( TokenIterator& TokenPosition )
{
    // read the instruction name
    // and use it to create a node
    Token* OpCodeToken = *TokenPosition;
    TokenPosition++;
    
    InstructionNode* NewNode = new InstructionNode;
    NewNode->Location = OpCodeToken->Location;
    NewNode->OpCode = ((InstructionOpCodeToken*)OpCodeToken)->Which;
    
    // read all parameters in the same line
    while( AreInSameLine( OpCodeToken, *TokenPosition ) )
    {
        // (1) parse the new operand
        NewNode->Operands.push_back( ParseOperand( TokenPosition ) );
        
        // (2) expect either a comma or a new line
        Token* CommaToken = *TokenPosition;
        
        if( !AreInSameLine(CommaToken, OpCodeToken) )
          break;
        
        if( !TokenIsThisSymbol( CommaToken, SymbolTypes::Comma ) )
          EmitError( OpCodeToken->Location, "expected comma separating instruction operands" );
        
        // consume the comma
        TokenPosition++;
        
        // expect that a next operand exists
        Token* NextOperandToken = *TokenPosition;
        
        if( NextOperandToken->Type() == TokenTypes::EndOfFile )
          EmitError( OpCodeToken->Location, "unexpected end of file" );
        
        // the next operand must be in the same line
        ExpectSameLine( OpCodeToken, NextOperandToken );
    }
    
    return NewNode;
}

// -----------------------------------------------------------------------------

IntegerDataNode* VirconASMParser::ParseIntegerData( TokenIterator& TokenPosition )
{
    // consume the definition keyword
    Token* KeywordToken = *TokenPosition;
    TokenPosition++;
    
    // create a node
    IntegerDataNode* NewNode = new IntegerDataNode;
    NewNode->Location = KeywordToken->Location;
    
    // read all values in the same line
    while( AreInSameLine( KeywordToken, *TokenPosition ) )
    {
        // (1) parse a value
        BasicValue Value = ParseBasicValue( TokenPosition );
        
        // (2) expect it to be a literal integer
        if( Value.Type != BasicValueTypes::LiteralInteger )
          EmitError( KeywordToken->Location, "expected a number literal" );
          
        else
          NewNode->Values.push_back( Value.IntegerField );
        
        // (3) expect either a comma or a new line
        Token* CommaToken = *TokenPosition;
        
        if( !AreInSameLine(CommaToken, KeywordToken) )
          break;
        
        if( !TokenIsThisSymbol( CommaToken, SymbolTypes::Comma ) )
          EmitError( KeywordToken->Location, "expected comma separating integer values" );
        
        // consume the comma
        TokenPosition++;
    }
    
    // ensure that there is at least 1 value
    if( NewNode->Values.size() == 0 )
      EmitError( KeywordToken->Location, "no integer literals were declared" );
    
    // add the new node to the tree
    return NewNode;
}

// -----------------------------------------------------------------------------

FloatDataNode* VirconASMParser::ParseFloatData( TokenIterator& TokenPosition )
{
    // consume the definition keyword
    Token* KeywordToken = *TokenPosition;
    TokenPosition++;
    
    // create a node
    FloatDataNode* NewNode = new FloatDataNode;
    NewNode->Location = KeywordToken->Location;
    
    // read all values in the same line
    while( AreInSameLine( KeywordToken, *TokenPosition ) )
    {
        // (1) parse a value
        BasicValue Value = ParseBasicValue( TokenPosition );
        
        // (2) expect it to be a literal number
        if( Value.Type == BasicValueTypes::LiteralFloat )
          NewNode->Values.push_back( Value.FloatField );
          
        else if( Value.Type == BasicValueTypes::LiteralInteger )
          NewNode->Values.push_back( Value.IntegerField );
        
        else
          EmitError( KeywordToken->Location, "expected a number literal" );
        
        // (3) expect either a comma or a new line
        Token* CommaToken = *TokenPosition;
        
        if( !AreInSameLine(KeywordToken,CommaToken) )
          break;
        
        if( !TokenIsThisSymbol( CommaToken, SymbolTypes::Comma ) )
          EmitError( KeywordToken->Location, "expected comma separating float values" );
        
        // consume the comma
        TokenPosition++;
    }
    
    // ensure that there is at least 1 value
    if( NewNode->Values.size() == 0 )
      EmitError( KeywordToken->Location, "no float literals were declared" );
    
    // add the new node to the tree
    return NewNode;
}

// -----------------------------------------------------------------------------

StringDataNode* VirconASMParser::ParseStringData( TokenIterator& TokenPosition )
{
    // consume the definition keyword
    Token* KeywordToken = *TokenPosition;
    TokenPosition++;
    
    // create a node
    StringDataNode* NewNode = new StringDataNode;
    NewNode->Location = KeywordToken->Location;
    
    // expect a string in the same line
    ExpectSameLine( KeywordToken, *TokenPosition );
    Token* ValueToken = *TokenPosition;
    
    if( ValueToken->Type() != TokenTypes::LiteralString )
      EmitError( KeywordToken->Location, "expected a string literal" );
      
    // now we can assign the value
    // (it is no problem if the string is empty)
    NewNode->Value = ((LiteralStringToken*)ValueToken)->Value;
    
    // expect an end of line
    TokenPosition++;
    ExpectEndOfLine( KeywordToken, *TokenPosition );
    
    // add the new node to the tree
    return NewNode;
}

// -----------------------------------------------------------------------------

PointerDataNode* VirconASMParser::ParsePointerData( TokenIterator& TokenPosition )
{
    // consume the definition keyword
    Token* KeywordToken = *TokenPosition;
    TokenPosition++;
    
    // create a node
    PointerDataNode* NewNode = new PointerDataNode;
    NewNode->Location = KeywordToken->Location;
    
    // read all values in the same line
    while( AreInSameLine( KeywordToken, *TokenPosition ) )
    {
        // (1) parse a value
        BasicValue Value = ParseBasicValue( TokenPosition );
        
        // (2) expect it to be a label
        if( Value.Type != BasicValueTypes::Label )
          EmitError( KeywordToken->Location, "expected a label" );
          
        else
          NewNode->LabelNames.push_back( Value.LabelField );
        
        // (3) expect either a comma or a new line
        Token* CommaToken = *TokenPosition;
        
        if( !AreInSameLine(CommaToken, KeywordToken) )
          break;
        
        if( !TokenIsThisSymbol( CommaToken, SymbolTypes::Comma ) )
          EmitError( KeywordToken->Location, "expected comma separating labels" );
        
        // consume the comma
        TokenPosition++;
    }
    
    // ensure that there is at least 1 value
    if( NewNode->LabelNames.size() == 0 )
      EmitError( KeywordToken->Location, "no pointers to labels were declared" );
    
    // add the new node to the tree
    return NewNode;
}

// -----------------------------------------------------------------------------

LabelNode* VirconASMParser::ParseLabel( TokenIterator& TokenPosition )
{
    // read the label name
    // and use it to create a node
    Token* NameToken = *TokenPosition;
    TokenPosition++;
    
    LabelNode* NewNode = new LabelNode;
    NewNode->Location = NameToken->Location;
    NewNode->Name = ((LabelToken*)NameToken)->Name;
    
    // expect a colon in the same line
    ExpectSameLine( NameToken, *TokenPosition );
    
    if( !TokenIsThisSymbol( (*TokenPosition), SymbolTypes::Colon ) )
      EmitError( NameToken->Location, "expected colon after label declaration" );
    
    // expect an end of line
    TokenPosition++;
    ExpectEndOfLine( NameToken, *TokenPosition );
    
    return NewNode;
}

// -----------------------------------------------------------------------------

DataFileNode* VirconASMParser::ParseDataFile( TokenIterator& TokenPosition )
{
    // consume the definition keyword
    Token* KeywordToken = *TokenPosition;
    TokenPosition++;
    
    // create a node
    DataFileNode* NewNode = new DataFileNode;
    NewNode->Location = KeywordToken->Location;
    
    // expect a string in the same line
    ExpectSameLine( KeywordToken, *TokenPosition );
    Token* ValueToken = *TokenPosition;
    
    if( ValueToken->Type() != TokenTypes::LiteralString )
      EmitError( KeywordToken->Location, "expected a string literal" );
      
    // now we can assign the value
    NewNode->FilePath = ((LiteralStringToken*)ValueToken)->Value;
    
    // ensure that the string has at least 1 character
    if( NewNode->FilePath.size() == 0 )
      EmitError( KeywordToken->Location, "file path is empty" );
    
    // expect an end of line
    TokenPosition++;
    ExpectEndOfLine( KeywordToken, *TokenPosition );
    
    // add the new node to the tree
    return NewNode;
}


// =============================================================================
//      VIRCON ASM PARSER: MAIN PARSER FUNCTION
// =============================================================================


void VirconASMParser::ParseTopLevel( TokenList& Tokens_ )
{
    // delete any previous results
    for( ASTNode* Node : ProgramAST )
      delete Node;
    
    ProgramAST.clear();
    
    for( Token* T : Tokens )
      delete T;
    
    Tokens.clear();
    
    // copy the source list, cloning all tokens
    // (to ensure we don't leave dangling pointers)
    for( Token* T : Tokens_ )
      Tokens.push_back( T->Clone() );
    
    // parse the whole token list
    TokenIterator TokenPosition = Tokens.begin();
    
    while( TokenPosition != Tokens.end() )
    {
        Token* NextToken = *TokenPosition;
        
        // Start and end tokens are ignored here.
        // They are only used by specialized parsers
        if( IsFirstToken( NextToken ) )
        {
            TokenPosition++;
            continue;
        }
        
        if( IsLastToken( NextToken ) )
          break;
        
        // CASE 1: CPU Instruction
        if( NextToken->Type() == TokenTypes::InstructionOpCode )
        {
            InstructionNode* ParsedInstruction = ParseInstruction( TokenPosition );
            ProgramAST.push_back( ParsedInstruction );
            continue;
        }
        
        // CASE 2: Label declaration
        if( NextToken->Type() == TokenTypes::Label )
        {
            LabelNode* ParsedLabel = ParseLabel( TokenPosition );
            ProgramAST.push_back( ParsedLabel );
            continue;
        }
        
        // CASE 3: Integer data
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Integer ) )
        {
            IntegerDataNode* ParsedData = ParseIntegerData( TokenPosition );
            ProgramAST.push_back( ParsedData );
            continue;
        }
        
        // CASE 4: Float data
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Float ) )
        {
            FloatDataNode* ParsedData = ParseFloatData( TokenPosition );
            ProgramAST.push_back( ParsedData );
            continue;
        }
        
        // CASE 5: String data
        if( TokenIsThisKeyword( NextToken, KeywordTypes::String ) )
        {
            StringDataNode* ParsedData = ParseStringData( TokenPosition );
            ProgramAST.push_back( ParsedData );
            continue;
        }
        
        // CASE 6: Pointer data
        if( TokenIsThisKeyword( NextToken, KeywordTypes::Pointer ) )
        {
            PointerDataNode* ParsedData = ParsePointerData( TokenPosition );
            ProgramAST.push_back( ParsedData );
            continue;
        }
        
        // CASE 7: Data file
        if( TokenIsThisKeyword( NextToken, KeywordTypes::DataFile ) )
        {
            DataFileNode* ParsedData = ParseDataFile( TokenPosition );
            ProgramAST.push_back( ParsedData );
            continue;
        }
        
        // other (not valid)
        EmitError( NextToken->Location, "invalid start of sentence: " + NextToken->ToString() );
    }
}
