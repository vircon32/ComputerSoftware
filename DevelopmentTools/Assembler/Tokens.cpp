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

string IncludeKeywordToken::ToString()
{
    return "Include keyword";
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
    Cloned->Location = Location;
    return Cloned;
}

// -----------------------------------------------------------------------------

Token* EndOfFileToken::Clone()
{
    EndOfFileToken* Cloned = new EndOfFileToken;
    Cloned->Location = Location;
    return Cloned;
}

// -----------------------------------------------------------------------------

Token* LiteralIntegerToken::Clone()
{
    return NewIntegerToken( Location, Value );
}

// -----------------------------------------------------------------------------

Token* LiteralFloatToken::Clone()
{
    return NewFloatToken( Location, Value );
}

// -----------------------------------------------------------------------------

Token* LiteralStringToken::Clone()
{
    return NewStringToken( Location, Value );
}

// -----------------------------------------------------------------------------

Token* LabelToken::Clone()
{
    return NewLabelToken( Location, Name );
}

// -----------------------------------------------------------------------------

Token* InstructionOpCodeToken::Clone()
{
    return NewOpCodeToken( Location, Which );
}

// -----------------------------------------------------------------------------

Token* CPURegisterToken::Clone()
{
    return NewRegisterToken( Location, Which );
}

// -----------------------------------------------------------------------------

Token* IOPortToken::Clone()
{
    return NewPortToken( Location, Which );
}

// -----------------------------------------------------------------------------

Token* IOPortValueToken::Clone()
{
    return NewPortValueToken( Location, Which );
}

// -----------------------------------------------------------------------------

Token* VariableToken::Clone()
{
    return NewVariableToken( Location, Name );
}

// -----------------------------------------------------------------------------

Token* IntegerKeywordToken::Clone()
{
    return NewIntegerKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* FloatKeywordToken::Clone()
{
    return NewFloatKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* StringKeywordToken::Clone()
{
    return NewStringKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* PointerKeywordToken::Clone()
{
    return NewPointerKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* DefineKeywordToken::Clone()
{
    return NewDefineKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* IncludeKeywordToken::Clone()
{
    return NewIncludeKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* DataFileKeywordToken::Clone()
{
    return NewDataFileKeywordToken( Location );
}

// -----------------------------------------------------------------------------

Token* CommaToken::Clone()
{
    return NewCommaToken( Location );
}

// -----------------------------------------------------------------------------

Token* ColonToken::Clone()
{
    return NewColonToken( Location );
}

// -----------------------------------------------------------------------------

Token* PlusToken::Clone()
{
    return NewPlusToken( Location );
}

// -----------------------------------------------------------------------------

Token* MinusToken::Clone()
{
    return NewMinusToken( Location );
}

// -----------------------------------------------------------------------------

Token* OpenBracketToken::Clone()
{
    return NewOpenBracketToken( Location );
}

// -----------------------------------------------------------------------------

Token* CloseBracketToken::Clone()
{
    return NewCloseBracketToken( Location );
}


// =============================================================================
//      TOKEN CREATION FUNCTIONS
// =============================================================================


LiteralIntegerToken* NewIntegerToken( SourceLocation Location, int32_t Value )
{
    LiteralIntegerToken* NewToken = new LiteralIntegerToken;
    NewToken->Location = Location;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralFloatToken* NewFloatToken( SourceLocation Location, float Value )
{
    LiteralFloatToken* NewToken = new LiteralFloatToken;
    NewToken->Location = Location;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralStringToken* NewStringToken( SourceLocation Location, std::string Value )
{
    LiteralStringToken* NewToken = new LiteralStringToken;
    NewToken->Location = Location;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

CPURegisterToken* NewRegisterToken( SourceLocation Location, CPURegisters Which )
{
    CPURegisterToken* NewToken = new CPURegisterToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

InstructionOpCodeToken* NewOpCodeToken( SourceLocation Location, InstructionOpCodes Which )
{
    InstructionOpCodeToken* NewToken = new InstructionOpCodeToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

IOPortToken* NewPortToken( SourceLocation Location, IOPorts Which )
{
    IOPortToken* NewToken = new IOPortToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

IOPortValueToken* NewPortValueToken( SourceLocation Location, IOPortValues Which )
{
    IOPortValueToken* NewToken = new IOPortValueToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

LabelToken* NewLabelToken( SourceLocation Location, string& Name )
{
    LabelToken* NewToken = new LabelToken;
    NewToken->Location = Location;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

VariableToken* NewVariableToken( SourceLocation Location, std::string& Name )
{
    VariableToken* NewToken = new VariableToken;
    NewToken->Location = Location;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

IntegerKeywordToken* NewIntegerKeywordToken( SourceLocation Location )
{
    IntegerKeywordToken* NewToken = new IntegerKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

FloatKeywordToken* NewFloatKeywordToken( SourceLocation Location )
{
    FloatKeywordToken* NewToken = new FloatKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

StringKeywordToken* NewStringKeywordToken( SourceLocation Location )
{
    StringKeywordToken* NewToken = new StringKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

PointerKeywordToken* NewPointerKeywordToken( SourceLocation Location )
{
    PointerKeywordToken* NewToken = new PointerKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

DefineKeywordToken* NewDefineKeywordToken( SourceLocation Location )
{
    DefineKeywordToken* NewToken = new DefineKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

IncludeKeywordToken* NewIncludeKeywordToken( SourceLocation Location )
{
    IncludeKeywordToken* NewToken = new IncludeKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

DataFileKeywordToken* NewDataFileKeywordToken( SourceLocation Location )
{
    DataFileKeywordToken* NewToken = new DataFileKeywordToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

CommaToken* NewCommaToken( SourceLocation Location )
{
    CommaToken* NewToken = new CommaToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

ColonToken* NewColonToken( SourceLocation Location )
{
    ColonToken* NewToken = new ColonToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

PlusToken* NewPlusToken( SourceLocation Location )
{
    PlusToken* NewToken = new PlusToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

MinusToken* NewMinusToken( SourceLocation Location )
{
    MinusToken* NewToken = new MinusToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

OpenBracketToken* NewOpenBracketToken( SourceLocation Location )
{
    OpenBracketToken* NewToken = new OpenBracketToken;
    NewToken->Location = Location;
    return NewToken;
}

// -----------------------------------------------------------------------------

CloseBracketToken* NewCloseBracketToken( SourceLocation Location )
{
    CloseBracketToken* NewToken = new CloseBracketToken;
    NewToken->Location = Location;
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

bool IsFirstToken( const TokenIterator& TokenPosition )
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

bool IsLastToken( const TokenIterator& TokenPosition )
{
    Token* CurrentToken = *TokenPosition;
    return (CurrentToken->Type() == TokenTypes::EndOfFile);
}

