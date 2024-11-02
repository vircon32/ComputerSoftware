// *****************************************************************************
    // start include guard
    #ifndef VIRCONCEMITTER_HPP
    #define VIRCONCEMITTER_HPP
    
    // include project headers
    #include "CNodes.hpp"
    #include "RegisterAllocation.hpp"
// *****************************************************************************


// =============================================================================
//      VIRCON C EMITTER
// =============================================================================


class VirconCEmitter
{
    protected:
        
        // link to source data
        TopLevelNode* ProgramAST;
        
    public:
        
        // results
        std::vector< std::string > ProgramLines;
        std::vector< std::string > DataLines;
        
        // debug info: C->ASM line correspondence
        std::map< int, CNode* > LineMapping;
        
    public:
        
        // called when emitting ASM to keep track of
        // C -> ASM line correspondence (for debug info)
        void AddDebugInfo( CNode* Node );
        
        // emit functions for abstract node types
        int EmitCNode( CNode* Node );
        int EmitRootExpression( ExpressionNode* Expression );
        void EmitDependentExpression( ExpressionNode* Expression, RegisterAllocation& Registers, int ResultRegister );
        
        // emit function for specific nodes (declarations)
        int EmitVariableList       ( VariableListNode* VariableList );
        int EmitVariable           ( VariableNode* Variable );
        int EmitFunction           ( FunctionNode* Function );
        int EmitEmbeddedFile       ( EmbeddedFileNode* EmbeddedFile );
        
        // specific emitters for variable initializations
        int EmitInitialization( MemoryPlacement LeftPlacement, DataType* LeftType, CNode* InitialValue );
        int EmitSingleInitialization( MemoryPlacement LeftPlacement, DataType* LeftType, ExpressionNode* Value );
        int EmitStringInitialization( MemoryPlacement LeftPlacement, LiteralStringNode* LiteralString );
        int EmitArrayInitialization( MemoryPlacement LeftPlacement, ArrayType* LeftType, InitializationListNode* ValueList );
        int EmitStructureInitialization( MemoryPlacement LeftPlacement, StructureNode* StructureDefinition, InitializationListNode* ValueList );
        
        // emit function for specific nodes (statements)
        int EmitIf                 ( IfNode* If );
        int EmitWhile              ( WhileNode* While );
        int EmitDo                 ( DoNode* Do );
        int EmitFor                ( ForNode* For );
        int EmitReturn             ( ReturnNode* Return );
        int EmitBreak              ( BreakNode* Break );
        int EmitContinue           ( ContinueNode* Continue );
        int EmitSwitch             ( SwitchNode* Switch );
        int EmitCase               ( CaseNode* Case );
        int EmitDefault            ( DefaultNode* Default );
        int EmitLabel              ( LabelNode* Label );
        int EmitGoto               ( GotoNode* Goto );
        int EmitBlock              ( BlockNode* Block );
        int EmitAssemblyBlock      ( AssemblyBlockNode* AssemblyBlock );
        
        // emit functions for specific expressions
        // (sizeof is not needed: it is always static)
        void EmitExpressionAtom     ( ExpressionAtomNode* ExpressionAtom          , RegisterAllocation& Registers, int ResultRegister );
        void EmitFunctionCall       ( FunctionCallNode* FunctionCall              , RegisterAllocation& Registers, int ResultRegister );
        void EmitArrayAccess        ( ArrayAccessNode* ArrayAccess                , RegisterAllocation& Registers, int ResultRegister );
        void EmitUnaryOperation     ( UnaryOperationNode* UnaryOperation          , RegisterAllocation& Registers, int ResultRegister );
        void EmitBinaryOperation    ( BinaryOperationNode* BinaryOperation        , RegisterAllocation& Registers, int ResultRegister );
        void EmitEnclosedExpression ( EnclosedExpressionNode* EnclosedExpression  , RegisterAllocation& Registers, int ResultRegister );
        void EmitMemberAccess       ( MemberAccessNode* MemberAccess              , RegisterAllocation& Registers, int ResultRegister );
        void EmitPointedMemberAccess( PointedMemberAccessNode* PointedMemberAccess, RegisterAllocation& Registers, int ResultRegister );
        void EmitLiteralString      ( LiteralStringNode* LiteralString            , RegisterAllocation& Registers, int ResultRegister );
        void EmitTypeConversion     ( TypeConversionNode* TypeConversion          , RegisterAllocation& Registers, int ResultRegister );
        
        // emit functions for individual unary operations
        void EmitPlusSign     ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitMinusSign    ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitPreIncrement ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitPreDecrement ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitPostIncrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitPostDecrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitLogicalNot   ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseNot   ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitReference    ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitDereference  ( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister );
        
        // emit functions for individual binary operations
        void EmitAddition             ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitSubtraction          ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitProduct              ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitDivision             ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitModulus              ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitEqual                ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitNotEqual             ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitLessThan             ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitLessOrEqual          ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitGreaterThan          ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitGreaterOrEqual       ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitLogicalOr            ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitLogicalAnd           ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseOr            ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseAnd           ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseXor           ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitShiftLeft            ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitShiftRight           ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitAssignment           ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitAdditionAssignment   ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitSubtractionAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitProductAssignment    ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitDivisionAssignment   ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitModulusAssignment    ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseAndAssignment ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseOrAssignment  ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitBitwiseXorAssignment ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitShiftLeftAssignment  ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        void EmitShiftRightAssignment ( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        
        // helper function for all compound assignments
        void EmitComplementaryAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister );
        
        // non-node emission functions
        void EmitLabel( const std::string& LabelName );
        void EmitRegisterTypeConversion( int RegisterNumber, PrimitiveTypes ProducedType, PrimitiveTypes NeededType );
        void EmitRegisterTypeConversion( int RegisterNumber, DataType* ProducedType, DataType* NeededType );
        void EmitGlobalScopeFunction();
        void EmitProgramStartSection();
        void EmitHardwareErrorVector();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // emission functions for memory addresses
        void EmitStaticPlacement( MemoryPlacement Placement, int ResultRegister );
        void EmitExpressionPlacement( ExpressionNode* Expression, RegisterAllocation& Registers, int ResultRegister );
        
    public:
        
        // instance handling
        VirconCEmitter();
        ~VirconCEmitter();
        
        // main emission function
        void Emit( TopLevelNode& ProgramAST_, bool IsBios );
        void SaveAssembly( const std::string& FilePath );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
