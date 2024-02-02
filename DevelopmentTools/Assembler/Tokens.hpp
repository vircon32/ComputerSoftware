// *****************************************************************************
    // start include guard
    #ifndef TOKENS_HPP
    #define TOKENS_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include project headers
    #include "SourceLocation.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <list>         // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      DEFINITIONS
// =============================================================================


enum class TokenTypes
{
    StartOfFile,        // artificial token, used as a marker
    EndOfFile,          // artificial token, used as a marker
    LiteralInteger,     // can be decimal or hexadecimal
    LiteralFloat,       // can be in scientific notation
    LiteralString,      // can contain escaped characters
    Label,              // named addresses
    Identifier,         // general identifier, to be recognized as a more specific token
    InstructionOpCode,  // mnemonics for all instructions
    CPURegister,        // all CPU elements: registers, etc
    IOPort,             // all I/O ports: GPU, Gamepads, ...
    IOPortValue,        // all predefined I/O port values: GPU/SPU commands, etc
    Keyword,            // all language-reserved keywords for statements, etc
    Symbol              // all delimiters and special symbols
};

// -----------------------------------------------------------------------------

enum class KeywordTypes
{
    Integer,     // statement to define non-executable data integers as literals
    Float,       // statement to define non-executable data floats as literals
    String,      // statement to define a non-executable data string as a literal
    Pointer,     // statement to define a non-executable data pointers as labels
    DataFile     // statement to insert data from another file
};

// -----------------------------------------------------------------------------

enum class SymbolTypes
{
    Comma,              // to separate operands
    Colon,              // to delimit labels
    Plus,               // to denote memory offsets
    Minus,              // to form negative numbers, or to denote memory offsets
    Percent,            // to form preprocessor directives
    OpenBracket,        // to enclose memory addresses
    CloseBracket        // to enclose memory addresses
};


// =============================================================================
//      CONVERSIONS: TOKEN TYPES <-> STRING
// =============================================================================


// detection from a string
bool IsKeyword( const std::string& Word );
bool IsSymbol( const std::string& Word );

// string --> token types
KeywordTypes WhichKeyword( const std::string& Name );
SymbolTypes WhichSymbol( const std::string& Name );

// token types --> string
std::string KeywordToString( KeywordTypes Which );
std::string SymbolToString( SymbolTypes Which );


// =============================================================================
//      BASE TOKEN CLASS
// =============================================================================


class Token
{
    public:
    
        SourceLocation Location;
        
    public:
        
        virtual ~Token() {};   // needed for base classes to be destructed
        virtual TokenTypes Type() = 0;
        virtual std::string ToString() = 0;
        virtual Token* Clone() = 0;
};

// -----------------------------------------------------------------------------

// from now on, use these for shorter function prototypes
typedef std::list< Token* > TokenList;
typedef std::list< Token* >::iterator TokenIterator;


// =============================================================================
//      DERIVED TOKEN CLASSES
// =============================================================================


class StartOfFileToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::StartOfFile; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class EndOfFileToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::EndOfFile; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class LiteralIntegerToken: public Token
{
    public:
        
        int32_t Value;
        
        virtual TokenTypes Type() { return TokenTypes::LiteralInteger; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class LiteralFloatToken: public Token
{
    public:
        
        float Value;
        
        virtual TokenTypes Type() { return TokenTypes::LiteralFloat; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class LiteralStringToken: public Token
{
    public:
        
        std::string Value;
        
        virtual TokenTypes Type() { return TokenTypes::LiteralString; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class LabelToken: public Token
{
    public:
        
        std::string Name;
        
        virtual TokenTypes Type() { return TokenTypes::Label; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class IdentifierToken: public Token
{
    public:
        
        std::string Name;
        
        virtual TokenTypes Type() { return TokenTypes::Identifier; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class InstructionOpCodeToken: public Token
{
    public:
        
        V32::InstructionOpCodes Which;
        
        virtual TokenTypes Type() { return TokenTypes::InstructionOpCode; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class CPURegisterToken: public Token
{
    public:
        
        V32::CPURegisters Which;
        
        virtual TokenTypes Type() { return TokenTypes::CPURegister; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class IOPortToken: public Token
{
    public:
        
        V32::IOPorts Which;
        
        virtual TokenTypes Type() { return TokenTypes::IOPort; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class IOPortValueToken: public Token
{
    public:
        
        V32::IOPortValues Which;
        
        virtual TokenTypes Type() { return TokenTypes::IOPortValue; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class KeywordToken: public Token
{
    public:
        
        KeywordTypes Which;
        
        virtual TokenTypes Type() { return TokenTypes::Keyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class SymbolToken: public Token
{
    public:
        
        SymbolTypes Which;
        
        virtual TokenTypes Type() { return TokenTypes::Symbol; }
        virtual std::string ToString();
        virtual Token* Clone();
};


// =============================================================================
//      TOKEN CREATION FUNCTIONS
// =============================================================================


// these would really only needed for tokens that use parameters
// but we want it for all of them to init their source location
LiteralIntegerToken* NewIntegerToken( SourceLocation Location, int32_t Value );
LiteralFloatToken* NewFloatToken( SourceLocation Location, float Value );
LiteralStringToken* NewStringToken( SourceLocation Location, std::string Value );
LabelToken* NewLabelToken( SourceLocation Location, std::string& Name );
IdentifierToken* NewIdentifierToken( SourceLocation Location, std::string& Name );
InstructionOpCodeToken* NewOpCodeToken( SourceLocation Location, V32::InstructionOpCodes Which );
CPURegisterToken* NewRegisterToken( SourceLocation Location, V32::CPURegisters Which );
IOPortToken* NewPortToken( SourceLocation Location, V32::IOPorts Which );
IOPortValueToken* NewPortValueToken( SourceLocation Location, V32::IOPortValues Which );
KeywordToken* NewKeywordToken( SourceLocation Location, KeywordTypes Which );
SymbolToken* NewSymbolToken( SourceLocation Location, SymbolTypes Which );


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsLastToken( Token* T );
bool IsLastToken( const TokenIterator& TokenPosition );

bool IsFirstToken( Token* T );
bool IsFirstToken( const TokenIterator& TokenPosition );

bool TokenIsThisKeyword( Token* T, KeywordTypes Which );
bool TokenIsThisSymbol( Token* T, SymbolTypes Which );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
