// *****************************************************************************
    // start include guard
    #ifndef TOKENS_HPP
    #define TOKENS_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
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
    IntegerKeyword,     // statement to define non-executable data integers
    FloatKeyword,       // statement to define non-executable data floats
    StringKeyword,      // statement to define a non-executable data string
    DefineKeyword,      // statement to define a variable
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
        
        int LineInSource;
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

class DefineKeywordToken: public Token
{
    public:
        
        virtual TokenTypes Type() { return TokenTypes::DefineKeyword; }
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
LiteralIntegerToken* NewIntegerToken( int LineNumber, int32_t Value );
LiteralFloatToken* NewFloatToken( int LineNumber, float Value );
LiteralStringToken* NewStringToken( int LineNumber, std::string Value );
LabelToken* NewLabelToken( int LineNumber, std::string& Name );
InstructionOpCodeToken* NewOpCodeToken( int LineNumber, V32::InstructionOpCodes Which );
CPURegisterToken* NewRegisterToken( int LineNumber, V32::CPURegisters Which );
IOPortToken* NewPortToken( int LineNumber, V32::IOPorts Which );
IOPortValueToken* NewPortValueToken( int LineNumber, V32::IOPortValues Which );
VariableToken* NewVariableToken( int LineNumber, std::string& Name );
IntegerKeywordToken* NewIntegerKeywordToken( int LineNumber );
FloatKeywordToken* NewFloatKeywordToken( int LineNumber );
StringKeywordToken* NewStringKeywordToken( int LineNumber );
DefineKeywordToken* NewDefineKeywordToken( int LineNumber );
DataFileKeywordToken* NewDataFileKeywordToken( int LineNumber );
CommaToken* NewCommaToken( int LineNumber );
ColonToken* NewColonToken( int LineNumber );
PlusToken* NewPlusToken( int LineNumber );
MinusToken* NewMinusToken( int LineNumber );
OpenBracketToken* NewOpenBracketToken( int LineNumber );
CloseBracketToken* NewCloseBracketToken( int LineNumber );


// =============================================================================
//      DETECTION OF SPECIFIC TOKENS
// =============================================================================


bool IsLastToken( Token* T );
bool IsLastToken( TokenIterator& TokenPosition );

bool IsFirstToken( Token* T );
bool IsFirstToken( TokenIterator& TokenPosition );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
