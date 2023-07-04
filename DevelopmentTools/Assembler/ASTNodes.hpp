// *****************************************************************************
    // start include guard
    #ifndef ASTNODES_HPP
    #define ASTNODES_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/Definitions.hpp"
    
    // include project headers
    #include "Tokens.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


// =============================================================================
//      DEFINITIONS
// =============================================================================


// A very simple AST with only 3 possible options.
// None can have children, so more a list than a tree
enum class ASTNodeTypes
{
    Instruction,
    IntegerData,
    FloatData,
    StringData,
    VariableDefinition,
    Label,
    DataFile
};


// =============================================================================
//      BASE AST NODE CLASS
// =============================================================================


class ASTNode
{
    public:
        
        int LineInSource;
        int AddressInROM;
        
    public:
        
        virtual ~ASTNode() {};
        virtual ASTNodeTypes Type() = 0;
        virtual std::string ToString() = 0;
};

// -----------------------------------------------------------------------------

// from now on use these for shorter function prototypes
typedef std::list< ASTNode* > NodeList;
typedef std::list< ASTNode* >::const_iterator NodeIterator;


// =============================================================================
//      GENERAL VALUE CLASS
// =============================================================================


enum class BasicValueTypes
{
    LiteralInteger,
    LiteralFloat,
    CPURegister,
    IOPort,
    IOPortValue,
    Label
};

// -----------------------------------------------------------------------------

// acts as a kind of "Variant" type
class BasicValue
{
    public:
        
        BasicValueTypes Type;
        
        int32_t             IntegerField;
        float               FloatField;
        V32::CPURegisters   RegisterField;
        V32::IOPorts        PortField;
        V32::IOPortValues   PortValueField;
        std::string         LabelField;
    
    public:
        
        BasicValue();
        std::string ToString();
};


// =============================================================================
//      OPERAND NODE CLASS
// =============================================================================


class InstructionOperand
{
    public:
        
        bool IsMemoryAddress;
        bool HasOffset;
        BasicValue Base;
        BasicValue Offset;
        
    public:
        
        InstructionOperand();
        std::string ToString();
};


// =============================================================================
//      DERIVED AST NODE CLASSES
// =============================================================================


class InstructionNode: public ASTNode
{
    public:
        
        V32::InstructionOpCodes OpCode;
        std::vector< InstructionOperand > Operands;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::Instruction; };
        virtual std::string ToString();
        
        // needed for ROM address allocation
        int SizeInWords();
};

// -----------------------------------------------------------------------------

class IntegerDataNode: public ASTNode
{
    public:
        
        std::vector< int32_t > Values;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::IntegerData; };
        virtual std::string ToString();
};

// -----------------------------------------------------------------------------

class FloatDataNode: public ASTNode
{
    public:
        
        std::vector< float > Values;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::FloatData; };
        virtual std::string ToString();
};

// -----------------------------------------------------------------------------

class StringDataNode: public ASTNode
{
    public:
        
        std::string Value;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::StringData; };
        virtual std::string ToString();
};

// -----------------------------------------------------------------------------

class VariableNode: public ASTNode
{
    public:
        
        std::string VariableName;
        Token* VariableValue;       // just a copied pointer
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::VariableDefinition; };
        virtual std::string ToString();
};

// -----------------------------------------------------------------------------

class LabelNode: public ASTNode
{
    public:
        
        std::string Name;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::Label; };
        virtual std::string ToString();
};

// -----------------------------------------------------------------------------

class DataFileNode: public ASTNode
{
    public:
        
        std::string FilePath;
        
        // needed for ROM address allocation
        std::vector< V32::V32Word > FileContents;
        
    public:
        
        virtual ASTNodeTypes Type() { return ASTNodeTypes::DataFile; };
        virtual std::string ToString();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
