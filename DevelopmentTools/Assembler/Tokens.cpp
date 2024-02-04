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
//      TOKEN TYPES --> STRING MAPS
// =============================================================================


const map< KeywordTypes, string > KeywordNames =
{
    { KeywordTypes::Integer,  "integer"  },
    { KeywordTypes::Float,    "float"    },
    { KeywordTypes::String,   "string"   },
    { KeywordTypes::Pointer,  "pointer"  },
    { KeywordTypes::DataFile, "datafile" }
};

// -----------------------------------------------------------------------------

const map< SymbolTypes, string > SymbolNames =
{
    { SymbolTypes::Comma,        "," },
    { SymbolTypes::Colon,        ":" },
    { SymbolTypes::Plus,         "+" },
    { SymbolTypes::Minus,        "-" },
    { SymbolTypes::Percent,      "%" },
    { SymbolTypes::OpenBracket,  "[" },
    { SymbolTypes::CloseBracket, "]" }
};


// =============================================================================
//      TOKEN TYPES: DETECTION FROM A STRING
// =============================================================================


bool IsKeyword( const std::string& Word )
{
    // search in the map
    for( auto MapPair : KeywordNames )
      if( MapPair.second == Word )
        return true;
    
    return false;
}

// -----------------------------------------------------------------------------

bool IsSymbol( const std::string& Word )
{
    // search in the map
    for( auto MapPair : SymbolNames )
      if( MapPair.second == Word )
        return true;
    
    return false;    
}


// =============================================================================
//      TOKEN TYPES: CONVERSION FROM STRING
// =============================================================================


KeywordTypes WhichKeyword( const string& Name )
{
    // search in the map
    for( auto MapPair : KeywordNames )
      if( MapPair.second == Name )
        return MapPair.first;
    
    // not found
    throw runtime_error( "string cannot be converted to a keyword" );
}

// -----------------------------------------------------------------------------

SymbolTypes WhichSymbol( const string& Name )
{
    // search in the map
    for( auto MapPair : SymbolNames )
      if( MapPair.second == Name )
        return MapPair.first;
    
    // not found
    throw runtime_error( "string cannot be converted to a symbol" );
}


// =============================================================================
//      TOKEN TYPES: CONVERSION TO STRING
// =============================================================================


string KeywordToString( KeywordTypes Which )
{
    // just search in the map
    auto MapPair = KeywordNames.find( Which );
    if( MapPair != KeywordNames.end() )
      return MapPair->second;
    
    // not found
    throw runtime_error( "keyword cannot be converted to a string" );
}

// -----------------------------------------------------------------------------

string SymbolToString( SymbolTypes Which )
{
    // just search in the map
    auto MapPair = SymbolNames.find( Which );
    if( MapPair != SymbolNames.end() )
      return MapPair->second;
    
    // not found
    throw runtime_error( "symbol cannot be converted to a string" );
}


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

string IdentifierToken::ToString()
{
    return string("Identifier: ") + Name;
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

string KeywordToken::ToString()
{
    return "Keyword " + KeywordToString( Which );
}

// -----------------------------------------------------------------------------

string SymbolToken::ToString()
{
    return "Symbol " + SymbolToString( Which );
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

Token* IdentifierToken::Clone()
{
    return NewIdentifierToken( Location, Name );
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

Token* KeywordToken::Clone()
{
    return NewKeywordToken( Location, Which );
}

// -----------------------------------------------------------------------------

Token* SymbolToken::Clone()
{
    return NewSymbolToken( Location, Which );
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

LabelToken* NewLabelToken( SourceLocation Location, string& Name )
{
    LabelToken* NewToken = new LabelToken;
    NewToken->Location = Location;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

IdentifierToken* NewIdentifierToken( SourceLocation Location, string& Name )
{
    IdentifierToken* NewToken = new IdentifierToken;
    NewToken->Location = Location;
    NewToken->Name = Name;
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

KeywordToken* NewKeywordToken( SourceLocation Location, KeywordTypes Which )
{
    KeywordToken* NewToken = new KeywordToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

SymbolToken* NewSymbolToken( SourceLocation Location, SymbolTypes Which )
{
    SymbolToken* NewToken = new SymbolToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
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

// -----------------------------------------------------------------------------

bool TokenIsThisKeyword( Token* T, KeywordTypes Which )
{
    if( T->Type() != TokenTypes::Keyword )
      return false;
    
    return ( ((KeywordToken*)T)->Which == Which );
}

// -----------------------------------------------------------------------------

bool TokenIsThisSymbol( Token* T, SymbolTypes Which )
{
    if( T->Type() != TokenTypes::Symbol )
      return false;
    
    return ( ((SymbolToken*)T)->Which == Which );
}


// =============================================================================
//      TRAVERSING OF TOKEN LISTS
// =============================================================================


TokenIterator Previous( const TokenIterator& TokenPosition )
{
    auto PreviousPosition = TokenPosition;
    PreviousPosition--;
    
    return PreviousPosition;
}

// -----------------------------------------------------------------------------

TokenIterator Next( const TokenIterator& TokenPosition )
{
    auto NextPosition = TokenPosition;
    NextPosition++;
    
    return NextPosition;
}

// -----------------------------------------------------------------------------

bool AreInSameLine( Token* T1, Token*T2 )
{
    // play safe
    if( !T1 || !T2 ) return false;
    
    // we will consider file limits as line changes too
    if( IsFirstToken(T1) || IsLastToken(T1) ) return false;
    if( IsFirstToken(T2) || IsLastToken(T2) ) return false;
    
    // now we can just compare
    return (T1->Location.Line == T2->Location.Line);
}
