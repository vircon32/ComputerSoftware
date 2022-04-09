// *****************************************************************************
    // start include guard
    #ifndef CTOKENS_HPP
    #define CTOKENS_HPP
    
    // include project headers
    #include "SourceLocation.hpp"
    
    // include C/C++ headers
    #include <list>         // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      TOKEN TYPE ENUMERATIONS
// =============================================================================


enum class CTokenTypes
{
    StartOfFile,
    EndOfFile,
    LiteralValue,
    LiteralString,
    Identifier,
    Keyword,
    Delimiter,
    Operator,
    SpecialSymbol
};

// -----------------------------------------------------------------------------

enum class LiteralValueTypes
{
    Bool,
    Int,
    Float
};

// -----------------------------------------------------------------------------

enum class KeywordTypes
{
    Void,
    Bool,
    Int,
    Float,
    If,
    Else,
    While,
    Do,
    For,
    Continue,
    Break,
    Return,
    Switch,
    Case,
    Default,
    Goto,
    Null,
    Struct,
    Union,
    Enum,
    SizeOf,
    Typedef,
    Asm,
    Embedded
};

// -----------------------------------------------------------------------------

enum class OperatorTypes
{
    Plus,
    Minus,
    Asterisk,
    Slash,
    Percent,            // Modulus,
    DoubleEqual,        // Equal,
    ExclamationEqual,   // NotEqual,
    Less,               // LessThan,
    LessEqual,          // LessOrEqual,
    Greater,            // GreaterThan,
    GreaterEqual,       // GreaterOrEqual,
    Exclamation,        // LogicalNot
    DoubleVerticalBar,  // LogicalOr,
    DoubleAmpersand,    // LogicalAnd,
    Tilde,              // BitwiseNot,
    VerticalBar,        // BitwiseOr,
    Ampersand,          // BitwiseAnd,
    Circumflex,         // BitwiseXor,
    DoubleLess,         // ShiftLeft,
    DoubleGreater,      // ShiftRight,
    Equal,              // Assignment,
    PlusEqual,
    MinusEqual,
    AsteriskEqual,
    SlashEqual,
    PercentEqual,       // ModulusEqual,
    AmpersandEqual,     // AndEqual,
    VerticalBarEqual,   // OrEqual,
    CircumflexEqual,    // XorEqual,
    DoubleLessEqual,    // ShiftLeftEqual,
    DoubleGreaterEqual, // ShiftRightEqual,
    DoublePlus,         // PreIncrement,
    DoubleMinus,        // PreDecrement
    Dot,
    MinusGreater
};

// -----------------------------------------------------------------------------

enum class DelimiterTypes
{
    OpenParenthesis,
    CloseParenthesis,
    OpenBrace,
    CloseBrace,
    OpenBracket,
    CloseBracket
};

// -----------------------------------------------------------------------------

enum class SpecialSymbolTypes
{
    Comma,
    Semicolon,
    Colon,
    Hash
};


// =============================================================================
//      CONVERSIONS: TOKEN TYPES <-> STRING
// =============================================================================


// detection from a string
bool IsKeyword( const std::string& Word );
bool IsOperator( const std::string& Word );
bool IsDelimiter( const char c );
bool IsSpecialSymbol( const char c );

// string --> token types
KeywordTypes WhichKeyword( const std::string& Name );
OperatorTypes WhichOperator( const std::string& Name );
DelimiterTypes WhichDelimiter( char c );
SpecialSymbolTypes WhichSpecialSymbol( char c );

// token types --> string
std::string KeywordToString( KeywordTypes Which );
std::string OperatorToString( OperatorTypes Which );
std::string DelimiterToString( DelimiterTypes Which );
std::string SpecialSymbolToString( SpecialSymbolTypes Which );


// =============================================================================
//      BASE TOKEN CLASS
// =============================================================================


class CToken
{
    public:
        
        SourceLocation Location;
        
    public:
        
        virtual ~CToken() {};   // needed for base classes to be destructed
        
        virtual CTokenTypes Type() = 0;
        virtual std::string ToString() = 0;
        virtual CToken* Clone() = 0;
};

// -----------------------------------------------------------------------------

// from now on, use these for shorter function prototypes
typedef std::list< CToken* > CTokenList;
typedef std::list< CToken* >::iterator CTokenIterator;


// =============================================================================
//      DERIVED TOKEN CLASSES
// =============================================================================


class StartOfFileToken: public CToken
{
    public:
        
        virtual CTokenTypes Type() { return CTokenTypes::StartOfFile; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class EndOfFileToken: public CToken
{
    public:
        
        virtual CTokenTypes Type() { return CTokenTypes::EndOfFile; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class IdentifierToken: public CToken
{
    public:
        
        std::string Name;
        
        virtual CTokenTypes Type() { return CTokenTypes::Identifier; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class KeywordToken: public CToken
{
    public:
        
        KeywordTypes Which;
        
        virtual CTokenTypes Type() { return CTokenTypes::Keyword; }
        virtual std::string ToString();
        virtual CToken* Clone();
        
        // keyword classification
        bool IsType();
};

// -----------------------------------------------------------------------------

class OperatorToken: public CToken
{
    public:
        
        OperatorTypes Which;
        
        virtual CTokenTypes Type() { return CTokenTypes::Operator; }
        virtual std::string ToString();
        virtual CToken* Clone();
        
        // operator classification
        bool IsUnary();
        bool IsBinary();
        
        // operator precedence (for binary operators only)
        int PrecedenceLevel();
};

// -----------------------------------------------------------------------------

class DelimiterToken: public CToken
{
    public:
        
        DelimiterTypes Which;
        
        virtual CTokenTypes Type() { return CTokenTypes::Delimiter; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class LiteralValueToken: public CToken
{
    public:
        
        LiteralValueTypes ValueType;
        bool BoolValue;
        int32_t IntValue;
        float FloatValue;
        
        virtual CTokenTypes Type() { return CTokenTypes::LiteralValue; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class LiteralStringToken: public CToken
{
    public:
        
        std::string Value;
        
        virtual CTokenTypes Type() { return CTokenTypes::LiteralString; }
        virtual std::string ToString();
        virtual CToken* Clone();
};

// -----------------------------------------------------------------------------

class SpecialSymbolToken: public CToken
{
    public:
        
        SpecialSymbolTypes Which;
        
        virtual CTokenTypes Type() { return CTokenTypes::SpecialSymbol; }
        virtual std::string ToString();
        virtual CToken* Clone();
};


// =============================================================================
//      TOKEN CREATION FUNCTIONS
// =============================================================================


LiteralValueToken* NewBoolToken( SourceLocation Location, bool Value );
LiteralValueToken* NewIntToken( SourceLocation Location, int Value );
LiteralValueToken* NewFloatToken( SourceLocation Location, float Value );
LiteralStringToken* NewStringToken( SourceLocation Location, std::string Value );
OperatorToken* NewOperatorToken( SourceLocation Location, OperatorTypes Which );
DelimiterToken* NewDelimiterToken( SourceLocation Location, DelimiterTypes Which );
KeywordToken* NewKeywordToken( SourceLocation Location, KeywordTypes Which );
IdentifierToken* NewIdentifierToken( SourceLocation Location, const std::string& Name );
SpecialSymbolToken* NewSpecialSymbolToken( SourceLocation Location, SpecialSymbolTypes Which );


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsFirstToken( CToken* T );
bool IsLastToken( CToken* T );

bool TokenIsThisKeyword( CToken* T, KeywordTypes Which );
bool TokenIsThisOperator( CToken* T, OperatorTypes Which );
bool TokenIsThisDelimiter( CToken* T, DelimiterTypes Which );
bool TokenIsThisSymbol( CToken* T, SpecialSymbolTypes Which );

bool TokenIsUnaryOperator( CToken* T );
bool TokenIsBinaryOperator( CToken* T );
bool TokenIsTypeKeyword( CToken* T );


// =============================================================================
//      TRAVERSING OF TOKEN LISTS
// =============================================================================


CTokenIterator Previous( const CTokenIterator& TokenPosition );
CTokenIterator Next( const CTokenIterator& TokenPosition );
bool AreInSameLine( CToken* T1, CToken*T2 );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

