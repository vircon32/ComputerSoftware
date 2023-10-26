// *****************************************************************************
    // include project headers
    #include "CTokens.hpp"
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <fstream>      // [ C++ STL ] File streams
    #include <map>          // [ C++ STL ] Maps
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      TOKEN TYPES --> STRING MAPS
// =============================================================================


const map< KeywordTypes, string > KeywordNames =
{
    { KeywordTypes::Void,     "void"     },
    { KeywordTypes::Bool,     "bool"     },
    { KeywordTypes::Int,      "int"      },
    { KeywordTypes::Float,    "float"    },
    { KeywordTypes::If,       "if"       },
    { KeywordTypes::Else,     "else"     },
    { KeywordTypes::While,    "while"    },
    { KeywordTypes::Do,       "do"       },
    { KeywordTypes::For,      "for"      },
    { KeywordTypes::Continue, "continue" },
    { KeywordTypes::Break,    "break"    },
    { KeywordTypes::Return,   "return"   },
    { KeywordTypes::Switch,   "switch"   },
    { KeywordTypes::Case,     "case"     },
    { KeywordTypes::Default,  "default"  },
    { KeywordTypes::Goto,     "goto"     },
    { KeywordTypes::Null,     "NULL"     },
    { KeywordTypes::Struct,   "struct"   },
    { KeywordTypes::Union,    "union"    },
    { KeywordTypes::Enum,     "enum"     },
    { KeywordTypes::SizeOf,   "sizeof"   },
    { KeywordTypes::Typedef,  "typedef"  },
    { KeywordTypes::Asm,      "asm"      },
    { KeywordTypes::Embedded, "embedded" },
    { KeywordTypes::Extern,   "extern"   }
};

// -----------------------------------------------------------------------------

const map< OperatorTypes, string > OperatorNames =
{
    { OperatorTypes::Plus,               "+"   },
    { OperatorTypes::Minus,              "-"   },
    { OperatorTypes::Slash,              "/"   },
    { OperatorTypes::Asterisk,           "*"   },
    { OperatorTypes::Percent,            "%"   },
    { OperatorTypes::DoubleEqual,        "=="  },
    { OperatorTypes::ExclamationEqual,   "!="  },
    { OperatorTypes::Less,               "<"   },
    { OperatorTypes::LessEqual,          "<="  },
    { OperatorTypes::Greater,            ">"   },
    { OperatorTypes::GreaterEqual,       ">="  },
    { OperatorTypes::Exclamation,        "!"   },
    { OperatorTypes::DoubleVerticalBar,  "||"  },
    { OperatorTypes::DoubleAmpersand,    "&&"  },
    { OperatorTypes::Tilde,              "~"   },
    { OperatorTypes::VerticalBar,        "|"   },
    { OperatorTypes::Ampersand,          "&"   },
    { OperatorTypes::Circumflex,         "^"   },
    { OperatorTypes::DoubleLess,         "<<"  },
    { OperatorTypes::DoubleGreater,      ">>"  },
    { OperatorTypes::Equal,              "="   },
    { OperatorTypes::PlusEqual,          "+="  },
    { OperatorTypes::MinusEqual,         "-="  },
    { OperatorTypes::AsteriskEqual,      "*="  },
    { OperatorTypes::SlashEqual,         "/="  },
    { OperatorTypes::PercentEqual,       "%="  },
    { OperatorTypes::AmpersandEqual,     "&="  },
    { OperatorTypes::VerticalBarEqual,   "|="  },
    { OperatorTypes::CircumflexEqual,    "^="  },
    { OperatorTypes::DoubleLessEqual,    "<<=" },
    { OperatorTypes::DoubleGreaterEqual, ">>=" },
    { OperatorTypes::DoublePlus,         "++"  },
    { OperatorTypes::DoubleMinus,        "--"  },
    { OperatorTypes::Dot,                "."   },
    { OperatorTypes::MinusGreater,       "->"  }
};

// -----------------------------------------------------------------------------

const map< DelimiterTypes, char > DelimiterNames =
{
    { DelimiterTypes::OpenParenthesis,  '(' },
    { DelimiterTypes::CloseParenthesis, ')' },
    { DelimiterTypes::OpenBrace,        '{' },
    { DelimiterTypes::CloseBrace,       '}' },
    { DelimiterTypes::OpenBracket,      '[' },
    { DelimiterTypes::CloseBracket,     ']' }
};

// -----------------------------------------------------------------------------

const map< SpecialSymbolTypes, char > SpecialSymbolNames =
{
    { SpecialSymbolTypes::Comma,     ',' },
    { SpecialSymbolTypes::Semicolon, ';' },
    { SpecialSymbolTypes::Colon,     ':' },
    { SpecialSymbolTypes::Hash,      '#' }
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

bool IsOperator( const std::string& Word )
{
    // search in the map
    for( auto MapPair : OperatorNames )
      if( MapPair.second == Word )
        return true;
    
    return false;    
}

// -----------------------------------------------------------------------------

bool IsDelimiter( const char c )
{
    // search in the map
    for( auto MapPair : DelimiterNames )
      if( MapPair.second == c )
        return true;
    
    return false;    
}

// -----------------------------------------------------------------------------

bool IsSpecialSymbol( const char c )
{
    // search in the map
    for( auto MapPair : SpecialSymbolNames )
      if( MapPair.second == c )
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

OperatorTypes WhichOperator( const string& Name )
{
    // search in the map
    for( auto MapPair : OperatorNames )
      if( MapPair.second == Name )
        return MapPair.first;
    
    // not found
    throw runtime_error( "string cannot be converted to an operator" );
}

// -----------------------------------------------------------------------------

DelimiterTypes WhichDelimiter( const char c )
{
    // search in the map
    for( auto MapPair : DelimiterNames )
      if( MapPair.second == c )
        return MapPair.first;
    
    // not found
    throw runtime_error( "character cannot be converted to a delimiter" );
}

// -----------------------------------------------------------------------------

SpecialSymbolTypes WhichSpecialSymbol( const char c )
{
    // search in the map
    for( auto MapPair : SpecialSymbolNames )
      if( MapPair.second == c )
        return MapPair.first;
    
    // not found
    throw runtime_error( "character cannot be converted to a special symbol" );
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

string OperatorToString( OperatorTypes Which )
{
    // just search in the map
    auto MapPair = OperatorNames.find( Which );
    if( MapPair != OperatorNames.end() )
      return MapPair->second;
    
    // not found
    throw runtime_error( "operator cannot be converted to a string" );
}

// -----------------------------------------------------------------------------

string DelimiterToString( DelimiterTypes Which )
{
    // just search in the map
    auto MapPair = DelimiterNames.find( Which );
    if( MapPair != DelimiterNames.end() )
    {
        string Result = string("'") + MapPair->second + "'";
        return Result;
    }
    
    // not found
    throw runtime_error( "delimiter cannot be converted to a string" );
}

// -----------------------------------------------------------------------------

string SpecialSymbolToString( SpecialSymbolTypes Which )
{
    // just search in the map
    auto MapPair = SpecialSymbolNames.find( Which );
    if( MapPair != SpecialSymbolNames.end() )
    {
        string Result = string("'") + MapPair->second + "'";
        return Result;
    }
    
    // not found
    throw runtime_error( "special symbol cannot be converted to a string" );
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

string LiteralValueToken::ToString()
{
    string Result = "Literal ";
    
    // distinguish types
    if( ValueType == LiteralValueTypes::Bool )
    {
        Result += "bool: ";
        Result += BoolValue? "true" : "false";
    }
    
    else if( ValueType == LiteralValueTypes::Int )
    {
        Result += "int: ";
        Result += to_string( IntValue );
    }
    
    else
    {
        Result += "float: ";
        Result += to_string( FloatValue );
    }
    
    return Result;
}

// -----------------------------------------------------------------------------

string LiteralStringToken::ToString()
{
    return string("Literal string: \"") + Value + "\"";
}

// -----------------------------------------------------------------------------

string IdentifierToken::ToString()
{
    return string("Identifier: ") + Name;
}

// -----------------------------------------------------------------------------

string KeywordToken::ToString()
{
    return string("Keyword: ") + KeywordToString( Which );
}

// -----------------------------------------------------------------------------

string DelimiterToken::ToString()
{
    return string("Delimiter: ") + DelimiterToString( Which );
}

// -----------------------------------------------------------------------------

string OperatorToken::ToString()
{
    return string("Operator: ") + OperatorToString( Which );
}

// -----------------------------------------------------------------------------

string SpecialSymbolToken::ToString()
{
    return string("Special symbol: ") + SpecialSymbolToString( Which );
}


// =============================================================================
//      TOKEN CLASSES: CLONING
// =============================================================================


CToken* StartOfFileToken::Clone()
{
    StartOfFileToken* Cloned = new StartOfFileToken;
    Cloned->Location = Location;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* EndOfFileToken::Clone()
{
    EndOfFileToken* Cloned = new EndOfFileToken;
    Cloned->Location = Location;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* LiteralValueToken::Clone()
{
    LiteralValueToken* Cloned = new LiteralValueToken;
    Cloned->Location = Location;
    Cloned->ValueType = ValueType;
    Cloned->BoolValue = BoolValue;
    Cloned->IntValue = IntValue;
    Cloned->FloatValue = FloatValue;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* LiteralStringToken::Clone()
{
    LiteralStringToken* Cloned = new LiteralStringToken;
    Cloned->Location = Location;
    Cloned->Value = Value;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* IdentifierToken::Clone()
{
    IdentifierToken* Cloned = new IdentifierToken;
    Cloned->Location = Location;
    Cloned->Name = Name;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* KeywordToken::Clone()
{
    KeywordToken* Cloned = new KeywordToken;
    Cloned->Location = Location;
    Cloned->Which = Which;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* OperatorToken::Clone()
{
    OperatorToken* Cloned = new OperatorToken;
    Cloned->Location = Location;
    Cloned->Which = Which;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* DelimiterToken::Clone()
{
    DelimiterToken* Cloned = new DelimiterToken;
    Cloned->Location = Location;
    Cloned->Which = Which;
    return Cloned;
}

// -----------------------------------------------------------------------------

CToken* SpecialSymbolToken::Clone()
{
    SpecialSymbolToken* Cloned = new SpecialSymbolToken;
    Cloned->Location = Location;
    Cloned->Which = Which;
    return Cloned;
}


// =============================================================================
//      TOKEN CLASSIFICATION
// =============================================================================


bool OperatorToken::IsUnary()
{
    if( Which == OperatorTypes::Plus        ) return true;
    if( Which == OperatorTypes::Minus       ) return true;
    if( Which == OperatorTypes::Exclamation ) return true;
    if( Which == OperatorTypes::Tilde       ) return true;
    if( Which == OperatorTypes::DoublePlus  ) return true; 
    if( Which == OperatorTypes::DoubleMinus ) return true;
    if( Which == OperatorTypes::Asterisk    ) return true;
    if( Which == OperatorTypes::Ampersand   ) return true;
    
    return false;
}

// -----------------------------------------------------------------------------

bool OperatorToken::IsBinary()
{
    // exclude unary-only operators
    if( Which == OperatorTypes::Exclamation ) return false;
    if( Which == OperatorTypes::Tilde       ) return false;
    if( Which == OperatorTypes::DoublePlus  ) return false; 
    if( Which == OperatorTypes::DoubleMinus ) return false;
    
    // exclude member access operators; although they are
    // in fact binary, they need to be treated differently
    // because they don't operate on values, but lexically
    if( Which == OperatorTypes::Dot          ) return false; 
    if( Which == OperatorTypes::MinusGreater ) return false;
    
    return true;
}

// -----------------------------------------------------------------------------

bool KeywordToken::IsType()
{
    if( Which == KeywordTypes::Int   ) return true;
    if( Which == KeywordTypes::Float ) return true;
    if( Which == KeywordTypes::Bool  ) return true;
    if( Which == KeywordTypes::Void  ) return true;
    
    // struct and union, unlike in regular C, are not
    // used to declare variables directly, but only to
    // define and name a type (as in a C++ class)
    return false;
}


// =============================================================================
//      TOKEN CREATION FUNCTIONS
// =============================================================================


LiteralValueToken* NewBoolToken( SourceLocation Location, bool Value )
{
    LiteralValueToken* NewToken = new LiteralValueToken;
    NewToken->Location = Location;
    NewToken->ValueType = LiteralValueTypes::Bool;
    NewToken->BoolValue = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralValueToken* NewIntToken( SourceLocation Location, int Value )
{
    LiteralValueToken* NewToken = new LiteralValueToken;
    NewToken->Location = Location;
    NewToken->ValueType = LiteralValueTypes::Int;
    NewToken->IntValue = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralValueToken* NewFloatToken( SourceLocation Location, float Value )
{
    LiteralValueToken* NewToken = new LiteralValueToken;
    NewToken->Location = Location;
    NewToken->ValueType = LiteralValueTypes::Float;
    NewToken->FloatValue = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

LiteralStringToken* NewStringToken( SourceLocation Location, string Value )
{
    LiteralStringToken* NewToken = new LiteralStringToken;
    NewToken->Location = Location;
    NewToken->Value = Value;
    return NewToken;
}

// -----------------------------------------------------------------------------

OperatorToken* NewOperatorToken( SourceLocation Location, OperatorTypes Which )
{
    OperatorToken* NewToken = new OperatorToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}

// -----------------------------------------------------------------------------

DelimiterToken* NewDelimiterToken( SourceLocation Location, DelimiterTypes Which )
{
    DelimiterToken* NewToken = new DelimiterToken;
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

IdentifierToken* NewIdentifierToken( SourceLocation Location, const string& Name )
{
    IdentifierToken* NewToken = new IdentifierToken;
    NewToken->Location = Location;
    NewToken->Name = Name;
    return NewToken;
}

// -----------------------------------------------------------------------------

SpecialSymbolToken* NewSpecialSymbolToken( SourceLocation Location, SpecialSymbolTypes Which )
{
    SpecialSymbolToken* NewToken = new SpecialSymbolToken;
    NewToken->Location = Location;
    NewToken->Which = Which;
    return NewToken;
}


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsFirstToken( CToken* T )
{
    return (T->Type() == CTokenTypes::StartOfFile);
}

// -----------------------------------------------------------------------------

bool IsLastToken( CToken* T )
{
    return (T->Type() == CTokenTypes::EndOfFile);
}

// -----------------------------------------------------------------------------

bool TokenIsThisKeyword( CToken* T, KeywordTypes Which )
{
    if( T->Type() != CTokenTypes::Keyword )
      return false;
    
    return ( ((KeywordToken*)T)->Which == Which );
}

// -----------------------------------------------------------------------------

bool TokenIsThisOperator( CToken* T, OperatorTypes Which )
{
    if( T->Type() != CTokenTypes::Operator )
      return false;
    
    return ( ((OperatorToken*)T)->Which == Which );
}

// -----------------------------------------------------------------------------

bool TokenIsThisDelimiter( CToken* T, DelimiterTypes Which )
{
    if( T->Type() != CTokenTypes::Delimiter )
      return false;
    
    return ( ((DelimiterToken*)T)->Which == Which );
}

// -----------------------------------------------------------------------------

bool TokenIsThisSymbol( CToken* T, SpecialSymbolTypes Which )
{
    if( T->Type() != CTokenTypes::SpecialSymbol )
      return false;
    
    return ( ((SpecialSymbolToken*)T)->Which == Which );
}

// -----------------------------------------------------------------------------

bool TokenIsUnaryOperator( CToken* T )
{
    if( T->Type() != CTokenTypes::Operator )
      return false;
    
    return ((OperatorToken*)T)->IsUnary();
}

// -----------------------------------------------------------------------------

bool TokenIsBinaryOperator( CToken* T )
{
    if( T->Type() != CTokenTypes::Operator )
      return false;
    
    return ((OperatorToken*)T)->IsBinary();
}

// -----------------------------------------------------------------------------

bool TokenIsTypeKeyword( CToken* T )
{
    if( T->Type() != CTokenTypes::Keyword )
      return false;
    
    return ((KeywordToken*)T)->IsType();
}


// =============================================================================
//      TRAVERSING OF TOKEN LISTS
// =============================================================================


CTokenIterator Previous( const CTokenIterator& TokenPosition )
{
    auto PreviousPosition = TokenPosition;
    PreviousPosition--;
    
    return PreviousPosition;
}

// -----------------------------------------------------------------------------

CTokenIterator Next( const CTokenIterator& TokenPosition )
{
    auto NextPosition = TokenPosition;
    NextPosition++;
    
    return NextPosition;
}

// -----------------------------------------------------------------------------

bool AreInSameLine( CToken* T1, CToken*T2 )
{
    // play safe
    if( !T1 || !T2 ) return false;
    
    // we will consider file limits as line changes too
    if( IsFirstToken(T1) || IsLastToken(T1) ) return false;
    if( IsFirstToken(T2) || IsLastToken(T2) ) return false;
    
    // now we can just compare
    return (T1->Location.LogicalLine == T2->Location.LogicalLine);
}

