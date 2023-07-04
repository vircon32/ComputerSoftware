// *****************************************************************************
    // start include guard
    #ifndef ASTNODES_HPP
    #define ASTNODES_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include project headers
    #include "CTokens.hpp"
    #include "DataTypes.hpp"
    #include "StaticValue.hpp"
    #include "Operators.hpp"
    #include "MemoryPlacement.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
    #include <map>              // [ C++ STL ] Maps
    #include <list>             // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      ENUMERATIONS OF C NODE TYPES
// =============================================================================


enum class CNodeTypes
{
    TopLevel,
    
    // declarations
    VariableList,
    Variable,
    InitializationList,
    Function,
    Structure,
    Union,
    MemberList,
    Member,
    Enumeration,
    EnumValue,
    Typedef,
    EmbeddedFile,
    
    // statements
    EmptyStatement,
    If,
    While,
    Do,
    For,
    Return,
    Break,
    Continue,
    Switch,
    Case,
    Default,
    Label,
    Goto,
    Block,
    AssemblyBlock,
    
    // expressions
    ExpressionAtom,
    FunctionCall,
    ArrayAccess,
    UnaryOperation,
    BinaryOperation,
    EnclosedExpression,
    MemberAccess,
    PointedMemberAccess,
    SizeOf,
    LiteralString,
    TypeConversion
};

// -----------------------------------------------------------------------------

enum class AtomTypes
{
    LiteralInteger,
    LiteralFloat,
    LiteralBoolean,
    EnumValue,
    Variable
};

// -----------------------------------------------------------------------------

enum class IdentifierTypes
{
    Function,
    Variable,
    Type,
    EnumValue
};

// -----------------------------------------------------------------------------

// expresses the node type in written form
std::string NodeTypeToLabel( CNodeTypes Type );


// =============================================================================
//      GENERIC C NODE INTERFACE
// =============================================================================


class ScopeNode;

class CNode
{
    public:
        
        // class-level info
        static int NextLabelNumber;
        
        // basic common metadata
        CNode* Parent;
        SourceLocation Location;
        int LabelNumber;
        
    public:
        
        // instance handling
        CNode( CNode* Parent_ );
        virtual ~CNode() {};
        
        // node classification
        virtual CNodeTypes Type() = 0;
        virtual bool IsExpression()         { return false; };
        virtual bool IsLoop()               { return false; };
        virtual bool IsType()               { return false; };
        virtual bool IsGroup()              { return false; };
        virtual bool IsScope()              { return false; };
        virtual bool HasStackFrame()        { return false; };
        virtual bool IsPartialDefinition()  { return false; };
        
        // log & debug
        virtual std::string ToXML() = 0;
        
        // help for node references
        ScopeNode* FindClosestScope( bool IncludeItself );
        
        // node identification in labels
        std::string NodeLabel();
};

// -----------------------------------------------------------------------------

// from now on use these for shorter function prototypes
typedef std::list< CNode* > CNodeList;
typedef std::list< CNode* >::iterator CNodeIterator;


// =============================================================================
//      SPECIALIZED ABSTRACT NODE INTERFACES
// =============================================================================


class ExpressionNode: public CNode
{
    public:

        DataType* ReturnedType;
        
    public:
        
        // instance handling
        ExpressionNode( CNode* Parent_ );
        virtual ~ExpressionNode();
        
        // node classification
        virtual CNodeTypes Type() = 0;
        virtual bool IsExpression() { return true; };
        
        // log & debug
        virtual std::string ToXML() = 0;
        
        // resulting value
        virtual bool IsStatic() = 0;
        virtual StaticValue GetStaticValue() = 0;
        virtual void DetermineReturnedType() = 0;
        virtual bool HasSideEffects() = 0;
        
        // resulting memory address
        virtual bool HasMemoryPlacement() = 0;
        virtual bool HasStaticPlacement() = 0;
        virtual MemoryPlacement GetStaticPlacement() = 0;
        
        // resource allocation
        virtual bool UsesFunctionCalls() = 0;
        virtual int SizeOfNeededTemporaries() = 0;
        void AllocateTemporaries();
};

// -----------------------------------------------------------------------------

// Nodes with a scope are able to define their own variables,
// but they don't create their own stack frame to store them.
// Instead, they are placed in the closest stack frame
class ScopeNode: public CNode
{
    public:
        
        // allocation of names (for identifiers
        // (of all types except goto labels)
        std::map< std::string, CNode* > DeclaredIdentifiers;
        
        // allocation of scope space in caller's stack frame
        int LocalVariablesOffset;    // base offset in owner stack frame
        int LocalVariablesSize;      // the sum of all locals
        
    public:
        
        // instance handling
        ScopeNode( CNode* Parent_ );
        virtual ~ScopeNode();
        
        // node classification
        virtual CNodeTypes Type() = 0;
        virtual bool IsScope() { return true; };
        
        // log & debug
        virtual std::string ToXML() = 0;
        
        // allocation and resolution of names
        CNode* ResolveIdentifier( std::string Name );
        void DeclareNewIdentifier( std::string Name, CNode* NewDeclaration );
        
        // allocation of variable space in caller's stack frame
        void AllocateVariablesInStack();
        void CalculateLocalVariablesOffset();
};

// -----------------------------------------------------------------------------

// Stack frame nodes can be thought of as nodes in the
// program's function call hierarchy: each one will
// preserve the caller's owned frame, and build their
// own execution context to support their processing
class StackFrameNode: public ScopeNode
{
    public:
        
        // stack frame layout
        int StackSizeForVariables;      // the maximum needed for all nested levels
        int StackSizeForTemporaries;    // as needed for complex expressions
        int StackSizeForFunctionCalls;  // the maximum of all them
        
    public:
        
        // instance handling
        StackFrameNode( CNode* Parent_ );
        virtual ~StackFrameNode();
        
        // node classification
        virtual bool HasStackFrame() { return true; };
        
        // stack frame properties
        int TotalStackSize();
};

// -----------------------------------------------------------------------------

class TypeNode: public CNode
{
    public:
        
        // node components
        std::string Name;
        DataType* DeclaredType;
        
        // external references
        ScopeNode* OwnerScope;
        
    public:
        
        // instance handling
        TypeNode( CNode* Parent_ );
        virtual ~TypeNode();
        
        // node classification
        virtual CNodeTypes Type() = 0;
        virtual bool IsType()  { return true; };
        
        // log & debug
        virtual std::string ToXML() = 0;
        
        // allocation in named definitions
        void AllocateName();
};

// -----------------------------------------------------------------------------

class MemberNode;

// groups define different parts within the whole
// structure that are identified by local names
class GroupNode: public TypeNode
{
    public:
        
        // node components
        std::map< std::string, MemberNode* > MembersByName;
        
    public:
        
        // instance handling
        GroupNode( CNode* Parent_ );
        virtual ~GroupNode();
        
        // node classification
        virtual CNodeTypes Type() = 0;
        virtual bool IsGroup()  { return true; };
        
        // log & debug
        virtual std::string ToXML() = 0;
        
        // allocation of names
        void DeclareNewMember( MemberNode* NewMember );
};


// =============================================================================
//      DECLARATION NODE CLASSES
// =============================================================================


class InitializationListNode;

class VariableNode: public CNode
{
    public:
        
        // node components
        std::string Name;
        DataType* DeclaredType;
        CNode* InitialValue;
        
        // external references
        ScopeNode* OwnerScope;
        bool IsReferenced;
        
        // allocation data
        bool IsArgument;
        int32_t OffsetInScope;       // needed to determine actual BP offset at emission
        MemoryPlacement Placement;
        
    public:
        
        // instance handling
        VariableNode( CNode* Parent_ );
        virtual ~VariableNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Variable; };
        
        // log & debug
        virtual std::string ToXML();
        
        // allocation as a resource
        void AllocateAsArgument();
        void AllocateAsVariable();
};

// -----------------------------------------------------------------------------

class VariableListNode: public CNode
{
    public:
        
        // variables
        std::list< VariableNode* > Variables;
        DataType* DeclaredType;
        
    public:
        
        // instance handling
        VariableListNode( CNode* Parent_ );
        virtual ~VariableListNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::VariableList; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

// Careful! This is not an expression. It may only be
// used in an initialization, but not in an assignment
class InitializationListNode: public CNode
{
    public:
        
        // node components
        // (values as CNode* because there can be nested initialization lists)
        std::list< CNode* > AssignedValues;
        
    public:
        
        // instance handling
        InitializationListNode( CNode* Parent_ );
        virtual ~InitializationListNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::InitializationList; };
        
        // log & debug
        virtual std::string ToXML();
        
        // list properties
        bool IsFullyStatic();
        int TotalSize();
};

// -----------------------------------------------------------------------------

class LabelNode;

class FunctionNode: public StackFrameNode
{
    public:
        
        // node components: function prototype
        std::string Name;
        DataType* ReturnType;
        std::list< VariableNode* > Arguments;
        
        // allocation of label names; C handles labels exclusively
        // at function level, and has a separate namespace for them
        std::map< std::string, LabelNode* > DeclaredLabels;
        
        // function body
        bool HasBody;
        std::list< CNode* > Statements;
        
        // allocation of function stack frame
        int SizeOfArguments;            // fixed size
        
    public:
        
        // instance handling
        FunctionNode( CNode* Parent_ );
        virtual ~FunctionNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Function; };
        virtual bool IsPartialDefinition() { return !HasBody; };
        
        // log & debug
        virtual std::string ToXML();
        
        // label names: allocation and resolution
        LabelNode* ResolveLabel( std::string Name );
        void DeclareNewLabel( LabelNode* NewLabel );
        
        // allocation in named definitions
        void AllocateName();
        bool PrototypeMatchesWith( FunctionNode* F2 );
};

// -----------------------------------------------------------------------------

// not the same as a variable!
// - it is not allocated in memory (only as an offset)
// - its name is allocated in a group, not in a scope
// - it is not independent (cannot be used in expressions)
// - it cannot have initialization
class MemberNode: public CNode
{
    public:
        
        // node components
        std::string Name;
        DataType* DeclaredType;
        
        // external references
        GroupNode* OwnerGroup;
        
        // allocation within owner group
        int OffsetInGroup;
        
    public:
        
        // instance handling
        MemberNode( CNode* Parent_ );
        virtual ~MemberNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Member; };
        
        // log & debug
        virtual std::string ToXML();
        
        // allocation as a resource
        void AllocateInGroup();
};

// -----------------------------------------------------------------------------

// not the same as a variable list!
// - its elements are not variables
class MemberListNode: public CNode
{
    public:
        
        // members
        std::list< MemberNode* > Members;
        DataType* DeclaredType;
        
    public:
        
        // instance handling
        MemberListNode(  CNode* Parent_ );
        virtual ~MemberListNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::MemberList; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class StructureNode: public GroupNode
{
    public:
        
        // node components
        std::list< MemberListNode* > Declarations;
        std::list< MemberNode* > MembersInOrder;
        bool HasBody;
        
        // local allocation
        int SizeOfMembers;
        
    public:
        
        // instance handling
        StructureNode( CNode* Parent_ );
        virtual ~StructureNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Structure; };
        virtual bool IsPartialDefinition() { return !HasBody; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

// careful! Union works direcly with members,
// since member lists are not allowed
class UnionNode: public GroupNode
{
    public:
        
        // node components
        std::list< MemberNode* > Declarations;
        bool HasBody;
        
        // local allocation
        int MaximumMemberSize;
        
    public:
        
        // instance handling
        UnionNode( CNode* Parent_ );
        virtual ~UnionNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Union; };
        virtual bool IsPartialDefinition() { return !HasBody; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class EnumerationNode;

class EnumValueNode: public CNode
{
    public:
        
        // node components
        std::string Name;
        int32_t Value;
        ExpressionNode* ValueExpression;
        
        // external references
        EnumerationNode* EnumerationContext;
        
    public:
        
        // instance handling
        EnumValueNode( CNode* Parent_ );
        virtual ~EnumValueNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::EnumValue; };
        
        // log & debug
        virtual std::string ToXML();
        
        // allocation as a resource
        void AllocateInEnum();
};

// -----------------------------------------------------------------------------

class EnumerationNode: public TypeNode
{
    public:
        
        // node components
        std::list< EnumValueNode* > Values;
        bool HasBody;
        
    public:
        
        // instance handling
        EnumerationNode( CNode* Parent_ );
        virtual ~EnumerationNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Enumeration; };
        virtual bool IsPartialDefinition() { return !HasBody; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class TypedefNode: public TypeNode
{
    public:
        
        // instance handling
        TypedefNode( CNode* Parent_ );
        virtual ~TypedefNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Typedef; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class EmbeddedFileNode: public CNode
{
    public:
        
        // node components
        std::string FilePath;
        VariableNode* Variable;
        
    public:
        
        // instance handling
        EmbeddedFileNode( CNode* Parent_ );
        virtual ~EmbeddedFileNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::EmbeddedFile; };
        
        // log & debug
        virtual std::string ToXML();
};


// =============================================================================
//      STATEMENT NODE CLASSES
// =============================================================================


class EmptyStatementNode: public CNode
{
    public:
        
        // instance handling
        EmptyStatementNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::EmptyStatement; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

// this class is only used on nested scopes
// (root scopes do have their own stack frame)
class BlockNode: public ScopeNode
{
    public:
        
        // block contents
        std::list< CNode* > Statements;
        
    public:
        
        // instance handling
        BlockNode( CNode* Parent_ );
        virtual ~BlockNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Block; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resource allocation
        void AllocateVariablesInFunction();
};

// -----------------------------------------------------------------------------

class ExpressionAtomNode;

class AssemblyBlockNode: public CNode
{
    public:
        
        struct AssemblyLine
        {
            std::string Text;
            ExpressionAtomNode* EmbeddedAtom;
        };
        
        // node components
        std::vector< AssemblyLine > AssemblyLines;
        
    public:
        
        // instance handling
        AssemblyBlockNode( CNode* Parent_ );
        virtual ~AssemblyBlockNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::AssemblyBlock; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class IfNode: public CNode
{
    public:
        
        // node components
        ExpressionNode* Condition;
        CNode* TrueStatement;
        CNode* FalseStatement;
        
    public:
        
        // instance handling
        IfNode( CNode* Parent_ );
        virtual ~IfNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::If; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class WhileNode: public CNode
{
    public:
        
        // node components
        ExpressionNode* Condition;
        CNode* LoopStatement;
        
    public:
        
        // instance handling
        WhileNode( CNode* Parent_ );
        virtual ~WhileNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::While; };
        virtual bool IsLoop()     { return true; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class DoNode: public CNode
{
    public:
        
        // node components
        ExpressionNode* Condition;
        CNode* LoopStatement;
        
    public:
        
        // instance handling
        DoNode( CNode* Parent_ );
        virtual ~DoNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Do; };
        virtual bool IsLoop()     { return true; };
        
        // log & debug
        virtual std::string ToXML();
};

// -----------------------------------------------------------------------------

class ForNode: public ScopeNode
{
    public:
        
        // node components
        CNode* InitialAction;
        ExpressionNode* Condition;
        ExpressionNode* IterationAction;
        CNode* LoopStatement;
        
    public:
        
        // instance handling
        ForNode( CNode* Parent_ );
        virtual ~ForNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::For; };
        virtual bool IsLoop()     { return true; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resource allocation
        void AllocateVariablesInFunction();
};

// -----------------------------------------------------------------------------

class ReturnNode: public CNode
{
    public:
        
        // node components
        ExpressionNode* ReturnedExpression;
        
        // external references
        FunctionNode* FunctionContext;
        
    public:
        
        // instance handling
        ReturnNode( CNode* Parent_ );
        virtual ~ReturnNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Return; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

class BreakNode: public CNode
{
    public:
        
        // external references
        CNode* Context;  // may be a loop, or a switch
        
    public:
        
        // instance handling
        BreakNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Break; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

class ContinueNode: public CNode
{
    public:
        
        // external references
        CNode* LoopContext;
        
    public:
        
        // instance handling
        ContinueNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Continue; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

// we can treat switch as a special kind of block,
// but it needs to have at least 1 case statement;
// also, variables cannot be declared as direct children
class CaseNode;
class DefaultNode;

class SwitchNode: public BlockNode
{
    public:
        
        // node components
        ExpressionNode* Condition;
        std::map< int, CaseNode* > HandledCases;
        DefaultNode* DefaultCase;
        
    public:
        
        // instance handling
        SwitchNode( CNode* Parent_ );
        virtual ~SwitchNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Switch; };
        
        // log & debug
        virtual std::string ToXML();
        
        // switch context specifics
        void AddCase( CaseNode* NewCase );
        void SetDefault( DefaultNode* NewDefault );
};

// -----------------------------------------------------------------------------

class CaseNode: public CNode
{
    public:
        
        // node components
        ExpressionNode* ValueExpression;
        int32_t Value;
        
        // external references
        SwitchNode* SwitchContext;
        
    public:
        
        // instance handling
        CaseNode( CNode* Parent_ );
        virtual ~CaseNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Case; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

class DefaultNode: public CNode
{
    public:
        
        // external references
        SwitchNode* SwitchContext;
        
    public:
        
        // instance handling
        DefaultNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Default; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

class LabelNode: public CNode
{
    public:
        
        // node components
        std::string Name;
        
        // external references
        FunctionNode* FunctionContext;
        
    public:
        
        // instance handling
        LabelNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Label; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};

// -----------------------------------------------------------------------------

class GotoNode: public CNode
{
    public:
        
        // node components
        std::string LabelName;
        
        // external references
        LabelNode* TargetLabel;
        FunctionNode* FunctionContext;
        
    public:
        
        // instance handling
        GotoNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::Goto; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveContext();
};


// =============================================================================
//      EXPRESSION NODE CLASSES
// =============================================================================


class ExpressionAtomNode: public ExpressionNode
{
    public:
        
        // node components
        AtomTypes    AtomType;
        bool         BoolValue;
        int32_t      IntValue;
        float        FloatValue;
        std::string  IdentifierName;
        
        // external references
        EnumValueNode* ResolvedEnumValue;
        VariableNode* ResolvedVariable;
        
    public:
        
        // instance handling
        ExpressionAtomNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::ExpressionAtom; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveIdentifier();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class FunctionCallNode: public ExpressionNode
{
    public:
        
        // node components
        std::string FunctionName;
        std::list< ExpressionNode* > Parameters;
        
        // external references
        FunctionNode* ResolvedFunction;
        
    public:
        
        // instance handling
        FunctionCallNode( CNode* Parent_ );
        virtual ~FunctionCallNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::FunctionCall; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveFunction();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
        void AllocateCallSpace();
};

// -----------------------------------------------------------------------------

class ArrayAccessNode: public ExpressionNode
{
    public:
        
        // node components
        ExpressionNode* ArrayOperand;
        ExpressionNode* IndexOperand;
        
    public:
        
        // instance handling
        ArrayAccessNode( CNode* Parent_ );
        virtual ~ArrayAccessNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::ArrayAccess; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class UnaryOperationNode: public ExpressionNode
{
    public:
        
        // node components
        UnaryOperators Operator;
        ExpressionNode* Operand;
        
    public:
        
        // instance handling
        UnaryOperationNode( CNode* Parent_ );
        virtual ~UnaryOperationNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::UnaryOperation; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class BinaryOperationNode: public ExpressionNode
{
    public:
        
        // node components
        BinaryOperators Operator;
        ExpressionNode* LeftOperand;
        ExpressionNode* RightOperand;
        
    public:
        
        // instance handling
        BinaryOperationNode( CNode* Parent_ );
        virtual ~BinaryOperationNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::BinaryOperation; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class EnclosedExpressionNode: public ExpressionNode
{
    public:
        
        // node components
        ExpressionNode* InternalExpression;
        
    public:
        
        // instance handling
        EnclosedExpressionNode( CNode* Parent_ );
        virtual ~EnclosedExpressionNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::EnclosedExpression; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class MemberAccessNode: public ExpressionNode
{
    public:
        
        // node components
        ExpressionNode* GroupOperand;
        std::string MemberName;
        
        // external references
        MemberNode* ResolvedMember;
        
    public:
        
        // instance handling
        MemberAccessNode( CNode* Parent_ );
        virtual ~MemberAccessNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::MemberAccess; };
        
        // log & debug
        virtual std::string ToXML();
             
        // resolve external references
        void ResolveMember();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();   
};

// -----------------------------------------------------------------------------

class PointedMemberAccessNode: public ExpressionNode
{
    public:
        
        // node components
        ExpressionNode* GroupOperand;
        std::string MemberName;
        
        // external references
        MemberNode* ResolvedMember;
        
    public:
        
        // instance handling
        PointedMemberAccessNode( CNode* Parent_ );
        virtual ~PointedMemberAccessNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::PointedMemberAccess; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resolve external references
        void ResolveMember();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class SizeOfNode: public ExpressionNode
{
    public:
        
        // node components
        DataType* QueriedType;
        ExpressionNode* QueriedExpression;
        
    public:
        
        // instance handling
        SizeOfNode( CNode* Parent_ );
        virtual ~SizeOfNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::SizeOf; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class LiteralStringNode: public ExpressionNode
{
    public:
        
        // node components
        std::string Value;
        
    public:
        
        // instance handling
        LiteralStringNode( CNode* Parent_ );
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::LiteralString; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};

// -----------------------------------------------------------------------------

class TypeConversionNode: public ExpressionNode
{
    public:
        
        // node components
        ExpressionNode* ConvertedExpression;
        DataType* RequestedType;
        
    public:
        
        // instance handling
        TypeConversionNode( CNode* Parent_ );
        virtual ~TypeConversionNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::TypeConversion; };
        
        // log & debug
        virtual std::string ToXML();
        
        // resulting value
        virtual bool IsStatic();
        virtual StaticValue GetStaticValue();
        virtual void DetermineReturnedType();
        virtual bool HasSideEffects();
        
        // resulting memory address
        virtual bool HasMemoryPlacement();
        virtual bool HasStaticPlacement();
        virtual MemoryPlacement GetStaticPlacement();
        
        // resource allocation
        virtual bool UsesFunctionCalls();
        virtual int SizeOfNeededTemporaries();
};


// =============================================================================
//      TOP-LEVEL NODE (AST ROOT)
// =============================================================================


class TopLevelNode: public StackFrameNode
{
    public:
        
        // program contents
        std::list< CNode* > Statements;
        
    public:
        
        // instance handling
        TopLevelNode();
        virtual ~TopLevelNode();
        
        // node classification
        virtual CNodeTypes Type() { return CNodeTypes::TopLevel; };
        
        // log & debug
        virtual std::string ToXML();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
