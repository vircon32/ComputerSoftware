// *****************************************************************************
    // start include guard
    #ifndef VIRCONCANALYZER_HPP
    #define VIRCONCANALYZER_HPP
    
    // include project headers
    #include "CNodes.hpp"
// *****************************************************************************


// =============================================================================
//      VIRCON C ANALYZER
// =============================================================================


// Analysis needs to do the following after parsing is complete:
// - determine returned type for all expressions
// - check parameters in function calls (both type and number)
// - check type compatibility in expressions
// - check that main function exists, and its prototype is correct
// - check that bios error handler function exists, and its prototype is correct
// - allocate all local variables in stack

class VirconCAnalyzer
{
    protected:
        
        // link to source data
        TopLevelNode* ProgramAST;
        
    public:
        
        // analysis functions for abstract node types
        void AnalyzeCNode( CNode* Node );
        void AnalyzeExpression( ExpressionNode* Expression, bool MustReturnValue );
        
        // analysis functions for specific nodes (declarations)
        void AnalyzeVariableList( VariableListNode* VariableList );
        void AnalyzeVariable( VariableNode* Variable );
        void AnalyzeFunction( FunctionNode* Function );
        void AnalyzeStructure( StructureNode* Structure );
        void AnalyzeUnion( UnionNode* Union );
        void AnalyzeEmbeddedFile( EmbeddedFileNode* EmbeddedFile );
        void AnalyzeInitializationList( InitializationListNode* InitializationList );
        
        // specific analyzers for variable initializations
        void AnalyzeInitialization( SourceLocation Location, DataType* VariableType, CNode* InitialValue );
        void AnalyzeStringInitialization( SourceLocation Location, int NumberOfElements, std::string Characters );
        void AnalyzeArrayInitialization( SourceLocation Location, ArrayType* LeftType, InitializationListNode* ValueList );
        void AnalyzeStructureInitialization( SourceLocation Location, StructureNode* StructureDefinition, InitializationListNode* ValueList );
        
        // analysis functions for specific nodes (statements)
        void AnalyzeBlock( BlockNode* Block );
        void AnalyzeIf( IfNode* If );
        void AnalyzeWhile( WhileNode* While );
        void AnalyzeDo( DoNode* Do );
        void AnalyzeFor( ForNode* For );
        void AnalyzeReturn( ReturnNode* Return );
        void AnalyzeBreak( BreakNode* Break );
        void AnalyzeContinue( ContinueNode* Continue );
        void AnalyzeSwitch( SwitchNode* Switch );
        void AnalyzeCase( CaseNode* Case );
        void AnalyzeDefault( DefaultNode* Default );
        void AnalyzeLabel( LabelNode* Label );
        void AnalyzeGoto( GotoNode* Goto );
        void AnalyzeAssemblyBlock( AssemblyBlockNode* AssemblyBlock );
        
        // non-node analysis functions
        void AnalyzeGlobalVariables();
        void AnalyzeFunctions();
        void AnalyzeMainFunction();
        void AnalyzeErrorHandlerFunction();
        
    public:
        
        // instance handling
        VirconCAnalyzer();
        ~VirconCAnalyzer();
        
        // main analysis function
        void Analyze( TopLevelNode& ProgramAST_, bool IsBios );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
