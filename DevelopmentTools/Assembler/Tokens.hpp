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
    InstructionOpCode,  // mnemonics for all instructions
    CPURegister,        // all CPU elements: registers, etc
    IOPort,             // all I/O ports: GPU, Gamepads, ...
    IOPortValue,        // all predefined I/O port values: GPU/SPU commands, etc
    Variable,           // variables declared to be replaced with a value
    IntegerKeyword,     // statement to define non-executable data integers as literals
    FloatKeyword,       // statement to define non-executable data floats as literals
    StringKeyword,      // statement to define a non-executable data string as a literal
    PointerKeyword,     // statement to define a non-executable data pointers as labels
    DefineKeyword,      // statement to define a variable
    IncludeKeyword,     // statement to include another file
    DataFileKeyword,    // statement to insert data from another file
    Comma,              // to separate operands
    Colon,              // to delimit labels
    Plus,               // to denote memory offsets
    Minus,              // to form negative numbers, or to denote memory offsets
    OpenBracket,        // to enclose memory addresses
    CloseBracket        // to enclose memory addresses
};


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

class VariableToken: public Token
{
    public:
        
        std::string Name;
        
        virtual TokenTypes Type() { return TokenTypes::Variable; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class IntegerKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::IntegerKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class FloatKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::FloatKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class StringKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::StringKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class PointerKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::PointerKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class DefineKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::DefineKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class IncludeKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::IncludeKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class DataFileKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::DataFileKeyword; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class CommaToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::Comma; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class ColonToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::Colon; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class PlusToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::Plus; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class MinusToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::Minus; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class OpenBracketToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::OpenBracket; }
        virtual std::string ToString();
        virtual Token* Clone();
};

// -----------------------------------------------------------------------------

class CloseBracketToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::CloseBracket; }
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
InstructionOpCodeToken* NewOpCodeToken( SourceLocation Location, V32::InstructionOpCodes Which );
CPURegisterToken* NewRegisterToken( SourceLocation Location, V32::CPURegisters Which );
IOPortToken* NewPortToken( SourceLocation Location, V32::IOPorts Which );
IOPortValueToken* NewPortValueToken( SourceLocation Location, V32::IOPortValues Which );
VariableToken* NewVariableToken( SourceLocation Location, std::string& Name );
IntegerKeywordToken* NewIntegerKeywordToken( SourceLocation Location );
FloatKeywordToken* NewFloatKeywordToken( SourceLocation Location );
StringKeywordToken* NewStringKeywordToken( SourceLocation Location );
PointerKeywordToken* NewPointerKeywordToken( SourceLocation Location );
DefineKeywordToken* NewDefineKeywordToken( SourceLocation Location );
IncludeKeywordToken* NewIncludeKeywordToken( SourceLocation Location );
DataFileKeywordToken* NewDataFileKeywordToken( SourceLocation Location );
CommaToken* NewCommaToken( SourceLocation Location );
ColonToken* NewColonToken( SourceLocation Location );
PlusToken* NewPlusToken( SourceLocation Location );
MinusToken* NewMinusToken( SourceLocation Location );
OpenBracketToken* NewOpenBracketToken( SourceLocation Location );
CloseBracketToken* NewCloseBracketToken( SourceLocation Location );


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsLastToken( Token* T );
bool IsLastToken( const TokenIterator& TokenPosition );

bool IsFirstToken( Token* T );
bool IsFirstToken( const TokenIterator& TokenPosition );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
