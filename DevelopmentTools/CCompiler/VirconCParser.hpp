// *****************************************************************************
    // start include guard
    #ifndef VIRCONCPARSER_HPP
    #define VIRCONCPARSER_HPP
    
    // include project headers
    #include "CNodes.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
// *****************************************************************************


// =============================================================================
//      TOKEN CLASSIFICATION
// =============================================================================


// used in situations where a given program element is expected
bool IsValidStartOfExpression( CToken* T );
bool IsValidStartOfStatement( CToken* T );


// =============================================================================
//      VIRCON C PARSER
// =============================================================================


class VirconCParser
{
    protected:
        
        // link to source data
        CTokenList* Tokens;
        
    public:
        
        // results
        TopLevelNode* ProgramAST;
        
    protected:
        
        // parsers for statements
        CNode* ParseStatement( CNode* Parent, CTokenIterator& TokenPosition, bool IsTopLevel );
        BlockNode* ParseBlock( CNode* Parent, CTokenIterator& TokenPosition );
        AssemblyBlockNode* ParseAssemblyBlock( CNode* Parent, CTokenIterator& TokenPosition );
        IfNode* ParseIf( CNode* Parent, CTokenIterator& TokenPosition );
        WhileNode* ParseWhile( CNode* Parent, CTokenIterator& TokenPosition );
        DoNode* ParseDo( CNode* Parent, CTokenIterator& TokenPosition );
        ForNode* ParseFor( CNode* Parent, CTokenIterator& TokenPosition );
        ReturnNode* ParseReturn( CNode* Parent, CTokenIterator& TokenPosition );
        BreakNode* ParseBreak( CNode* Parent, CTokenIterator& TokenPosition );
        ContinueNode* ParseContinue( CNode* Parent, CTokenIterator& TokenPosition );
        SwitchNode* ParseSwitch( CNode* Parent, CTokenIterator& TokenPosition );
        CaseNode* ParseCase( CNode* Parent, CTokenIterator& TokenPosition );
        DefaultNode* ParseDefault( CNode* Parent, CTokenIterator& TokenPosition );
        LabelNode* ParseLabel( CNode* Parent, CTokenIterator& TokenPosition );
        GotoNode* ParseGoto( CNode* Parent, CTokenIterator& TokenPosition );
        
        // parsers for declarations
        DataType* ParseType( CNode* Parent, CTokenIterator& TokenPosition );
        VariableNode* ParseFunctionArgument( FunctionNode* Function, CTokenIterator& TokenPosition );
        void ParseFunctionBody( FunctionNode* Function, CTokenIterator& TokenPosition );
        CNode* ParseDeclaration( CNode* Parent, CTokenIterator& TokenPosition, bool IsTopLevel );
        FunctionNode* ParseFunction( DataType* ReturnType, const std::string& Name, CNode* Parent, CTokenIterator& TokenPosition );
        VariableListNode* ParseVariableList( DataType* DeclaredType, const std::string& Name, bool UsesExtern, CNode* Parent, CTokenIterator& TokenPosition );
        VariableListNode* ParseExternVariableList( CNode* Parent, CTokenIterator& TokenPosition );
        InitializationListNode* ParseInitializationList( CNode* Parent, CTokenIterator& TokenPosition );
        MemberNode* ParseMember( UnionNode* OwnerUnion, CTokenIterator& TokenPosition );
        MemberListNode* ParseMemberList( StructureNode* OwnerStructure, CTokenIterator& TokenPosition );
        StructureNode* ParseStructure( CNode* Parent, CTokenIterator& TokenPosition );
        UnionNode* ParseUnion( CNode* Parent, CTokenIterator& TokenPosition );
        EnumValueNode* ParseEnumValue( CNode* Parent, CTokenIterator& TokenPosition );
        EnumerationNode* ParseEnumeration( CNode* Parent, CTokenIterator& TokenPosition );
        TypedefNode* ParseTypedef( CNode* Parent, CTokenIterator& TokenPosition );
        EmbeddedFileNode* ParseEmbeddedFile( CNode* Parent, CTokenIterator& TokenPosition );
        
        // parsers for expressions
        ExpressionNode* ParseExpression( CNode* Parent, CTokenIterator& TokenPosition, bool Greedy = true );
        ExpressionAtomNode* ParseExpressionAtom( CNode* Parent, CTokenIterator& TokenPosition );
        FunctionCallNode* ParseFunctionCall( CNode* Parent, CTokenIterator& TokenPosition );
        ArrayAccessNode* ParseArrayAccess( CNode* Parent, ExpressionNode* ArrayOperand, CTokenIterator& TokenPosition );
        UnaryOperationNode* ParseUnaryOperation( CNode* Parent, CTokenIterator& TokenPosition );
        BinaryOperationNode* ParseBinaryOperation( CNode* Parent, ExpressionNode* LeftOperand, OperatorToken* Operator, CTokenIterator& TokenPosition );
        EnclosedExpressionNode* ParseEnclosedExpression( CNode* Parent, CTokenIterator& TokenPosition );
        MemberAccessNode* ParseMemberAccess( CNode* Parent, ExpressionNode* LeftOperand, CTokenIterator& TokenPosition );
        PointedMemberAccessNode* ParsePointedMemberAccess( CNode* Parent, ExpressionNode* LeftOperand, CTokenIterator& TokenPosition );
        SizeOfNode* ParseSizeOf( CNode* Parent, CTokenIterator& TokenPosition );
        LiteralStringNode* ParseLiteralString( CNode* Parent, CTokenIterator& TokenPosition );
        TypeConversionNode* ParseTypeConversion( CNode* Parent, CTokenIterator& TokenPosition );
        
        // specifics for binary operations
        // (implementing operator precedence and associativity)
        BinaryOperationNode* AddBinaryOperationToTree( BinaryOperationNode* PreviousTree, OperatorToken* AddedOperator, ExpressionNode* AddedOperand );
        
    public:
        
        // instance handling
        VirconCParser();
       ~VirconCParser();
        
        // main parsing function
        void ParseTopLevel( CTokenList& Tokens_ );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
