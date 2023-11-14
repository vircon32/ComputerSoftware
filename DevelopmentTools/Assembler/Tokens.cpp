// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    
    // include project headers
    #include "Tokens.hpp"
    #include "Definitions.hpp"
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <fstream>      // [ C++ STL ] File streams
    #include <map>          // [ C++ STL ] Maps
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      TOKEN CLASSES: STRING CONVERSIONS
// =============================================================================


string StartOfFileToken::ToString()
{
    return "[START OF FILE]";
}

// -----------------------------------------------------------------------------

string EndOfFileToken::ToString()
{
    return "[END OF FILE]";
}

// -----------------------------------------------------------------------------

string LiteralIntegerToken::ToString()
{
    return string("Literal integer: ") + to_string( Value ) + " = " + Hex(Value,8);
}

// -----------------------------------------------------------------------------

string LiteralFloatToken::ToString()
{
    return string("Literal float: ") + to_string( Value );
}

// -----------------------------------------------------------------------------

string LiteralStringToken::ToString()
{
    return string("Literal string: \"") + Value + "\"";
}

// -----------------------------------------------------------------------------

string LabelToken::ToString()
{
    return string("Label: ") + Name;
}

// -----------------------------------------------------------------------------

string InstructionOpCodeToken::ToString()
{
    return string("Instruction: ") + OpCodeToString( Which );
}

// -----------------------------------------------------------------------------

string CPURegisterToken::ToString()
{
    return string("Register: ") + RegisterToString( Which );
}

// -----------------------------------------------------------------------------

string IOPortToken::ToString()
{
    return string("Port: ") + PortToString( Which );
}

// -----------------------------------------------------------------------------

string IOPortValueToken::ToString()
{
    return string("Port value: ") + PortValueToString( Which );
}

// -----------------------------------------------------------------------------

string VariableToken::ToString()
{
    return string("Variable: ") + Name;
}

// -----------------------------------------------------------------------------

string IntegerKeywordToken::ToString()
{
    return "Integer keyword";
}

// -----------------------------------------------------------------------------

string FloatKeywordToken::ToString()
{
    return "Float keyword";
}

// -----------------------------------------------------------------------------

string StringKeywordToken::ToString()
{
    return "String keyword";
}

// -----------------------------------------------------------------------------

string PointerKeywordToken::ToString()
{
    return "Pointer keyword";
}

// -----------------------------------------------------------------------------

string DefineKeywordToken::ToString()
{
    return "Define keyword";
}

// -----------------------------------------------------------------------------

string DataFileKeywordToken::ToString()
{
    return "DataFile keyword";
}

// -----------------------------------------------------------------------------

string CommaToken::ToString()
{
    return "Comma";
}

// -----------------------------------------------------------------------------

string ColonToken::ToString()
{
    return "Colon";
}

// -----------------------------------------------------------------------------

string PlusToken::ToString()
{
    return "Plus";
}

// -----------------------------------------------------------------------------

string MinusToken::ToString()
{
    return "Minus";
}

// -----------------------------------------------------------------------------

string OpenBracketToken::ToString()
{
    return "Open Bracket";
}

// -----------------------------------------------------------------------------

string CloseBracketToken::ToString()
{
    return "Close Bracket";
}


// =============================================================================
//      TOKEN CLASSES: CLONING
// =============================================================================


Token* StartOfFileToken::Clone()
{
    StartOfFileToken* Cloned = new StartOfFileToken;
    Cloned->LineInSource = LineInSource;
    return Cloned;
}

// -----------------------------------------------------------------------------

Token* EndOfFileToken::Clone()
{
    EndOfFileToken* Cloned = new EndOfFileToken;
    Cloned->LineInSource = LineInSource;
    return Cloned;
}

// -----------------------------------------------------------------------------

Token* LiteralIntegerToken::Clone()
{
    return NewIntegerToken( LineInSource, Value );
}

// -----------------------------------------------------------------------------

Token* LiteralFloatToken::Clone()
{
    return NewFloatToken( LineInSource, Value );
}

// -----------------------------------------------------------------------------

Token* LiteralStringToken::Clone()
{
    return NewStringToken( LineInSource, Value );
}

// -----------------------------------------------------------------------------

Token* LabelToken::Clone()
{
    return NewLabelToken( LineInSource, Name );
}

// -----------------------------------------------------------------------------

Token* InstructionOpCodeToken::Clone()
{
    return NewOpCodeToken( LineInSource, Which );
}

// -----------------------------------------------------------------------------

Token* CPURegisterToken::Clone()
{
    return NewRegisterToken( LineInSource, Which );
}

// -----------------------------------------------------------------------------

Token* IOPortToken::Clone()
{
    return NewPortToken( LineInSource, Which );
}

// -----------------------------------------------------------------------------

Token* IOPortValueToken::Clone()
{
    return NewPortValueToken( LineInSource, Which );
}

// -----------------------------------------------------------------------------

Token* VariableToken::Clone()
{
    return NewVariableToken( LineInSource, Name );
}

// -----------------------------------------------------------------------------

Token* IntegerKeywordToken::Clone()
{
    return NewIntegerKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* FloatKeywordToken::Clone()
{
    return NewFloatKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* StringKeywordToken::Clone()
{
    return NewStringKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* PointerKeywordToken::Clone()
{
    return NewPointerKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* DefineKeywordToken::Clone()
{
    return NewDefineKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* DataFileKeywordToken::Clone()
{
    return NewDataFileKeywordToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* CommaToken::Clone()
{
    return NewCommaToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* ColonToken::Clone()
{
    return NewColonToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* PlusToken::Clone()
{
    return NewPlusToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* MinusToken::Clone()
{
    return NewMinusToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* OpenBracketToken::Clone()
{
    return NewOpenBracketToken( LineInSource );
}

// -----------------------------------------------------------------------------

Token* CloseBracketToken::Clone()
{
    return NewCloseBracketToken( LineInSource );
}


// =============================================================================
//      TOKEN CREATION FUNCTIONS
// =============================================================================


LiteralIntegerToken* NewIntegerToken( int LineNumber, int32_t Value )
{
    LiteralIntegerToken* NewToken = new LiteralIntegerToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralFloatToken* NewFloatToken( int LineNumber, float Value )
{
    LiteralFloatToken* NewToken = new LiteralFloatToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralStringToken* NewStringToken( int LineNumber, std::string Value )
{
    LiteralStringToken* NewToken = new LiteralStringToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

CPURegisterToken* NewRegisterToken( int LineNumber, CPURegisters Which )
{
    CPURegisterToken* NewToken = new CPURegisterToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

InstructionOpCodeToken* NewOpCodeToken( int LineNumber, InstructionOpCodes Which )
{
    InstructionOpCodeToken* NewToken = new InstructionOpCodeToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

IOPortToken* NewPortToken( int LineNumber, IOPorts Which )
{
    IOPortToken* NewToken = new IOPortToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

IOPortValueToken* NewPortValueToken( int LineNumber, IOPortValues Which )
{
    IOPortValueToken* NewToken = new IOPortValueToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

LabelToken* NewLabelToken( int LineNumber, string& Name )
{
    LabelToken* NewToken = new LabelToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

VariableToken* NewVariableToken( int LineNumber, std::string& Name )
{
    VariableToken* NewToken = new VariableToken;
    NewToken->LineInSource = LineNumber;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

IntegerKeywordToken* NewIntegerKeywordToken( int LineNumber )
{
    IntegerKeywordToken* NewToken = new IntegerKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

FloatKeywordToken* NewFloatKeywordToken( int LineNumber )
{
    FloatKeywordToken* NewToken = new FloatKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

StringKeywordToken* NewStringKeywordToken( int LineNumber )
{
    StringKeywordToken* NewToken = new StringKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

PointerKeywordToken* NewPointerKeywordToken( int LineNumber )
{
    PointerKeywordToken* NewToken = new PointerKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

DefineKeywordToken* NewDefineKeywordToken( int LineNumber )
{
    DefineKeywordToken* NewToken = new DefineKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

DataFileKeywordToken* NewDataFileKeywordToken( int LineNumber )
{
    DataFileKeywordToken* NewToken = new DataFileKeywordToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

CommaToken* NewCommaToken( int LineNumber )
{
    CommaToken* NewToken = new CommaToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

ColonToken* NewColonToken( int LineNumber )
{
    ColonToken* NewToken = new ColonToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

PlusToken* NewPlusToken( int LineNumber )
{
    PlusToken* NewToken = new PlusToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

MinusToken* NewMinusToken( int LineNumber )
{
    MinusToken* NewToken = new MinusToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

OpenBracketToken* NewOpenBracketToken( int LineNumber )
{
    OpenBracketToken* NewToken = new OpenBracketToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}

// -----------------------------------------------------------------------------

CloseBracketToken* NewCloseBracketToken( int LineNumber )
{
    CloseBracketToken* NewToken = new CloseBracketToken;
    NewToken->LineInSource = LineNumber;
    return NewToken;
}


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsFirstToken( Token* T )
{
    return (T->Type() == TokenTypes::StartOfFile);
}

// -----------------------------------------------------------------------------

bool IsFirstToken( TokenIterator& TokenPosition )
{
    Token* CurrentToken = *TokenPosition;
    return (CurrentToken->Type() == TokenTypes::StartOfFile);
}

// -----------------------------------------------------------------------------

bool IsLastToken( Token* T )
{
    return (T->Type() == TokenTypes::EndOfFile);
}

// -----------------------------------------------------------------------------

bool IsLastToken( TokenIterator& TokenPosition )
{
    Token* CurrentToken = *TokenPosition;
    return (CurrentToken->Type() == TokenTypes::EndOfFile);
}

