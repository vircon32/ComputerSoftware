// *****************************************************************************
    // include project headers
    #include "VirconCAnalyzer.hpp"
    #include "CheckNodes.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      VIRCON C ANALYZER: INSTANCE HANDLING
// =============================================================================


VirconCAnalyzer::VirconCAnalyzer()
{
    ProgramAST = nullptr;
}

// -----------------------------------------------------------------------------

VirconCAnalyzer::~VirconCAnalyzer()
{
    // (do nothing)
}


// =============================================================================
//      VIRCON C ANALYZER: ANALYSIS FUNCTIONS FOR ABSTRACT NODE TYPES
// =============================================================================


void VirconCAnalyzer::AnalyzeCNode( CNode* Node )
{
    // some contructs have optional parts!
    // nothing to do if they are not used
    if( !Node ) return;
    
    // separate expressions from the rest
    // (they have their own general analyzer)
    if( Node->IsExpression() )
    {
        AnalyzeExpression( (ExpressionNode*)Node, false );
        return;
    }
    
    // for non-expression nodes
    switch( Node->Type() )
    {
        // declarations
        case CNodeTypes::VariableList:
            AnalyzeVariableList( (VariableListNode*)Node );
            return;
        case CNodeTypes::Function:
            AnalyzeFunction( (FunctionNode*)Node );
            return;
        case CNodeTypes::Structure:
            AnalyzeStructure( (StructureNode*)Node );
            return;
        case CNodeTypes::Union:
            AnalyzeUnion( (UnionNode*)Node );
            return;
        case CNodeTypes::EmbeddedFile:
            AnalyzeEmbeddedFile( (EmbeddedFileNode*)Node );
            return;
        
        // for these type declarations, no analysis
        // is needed (they only need to be parsed)
        case CNodeTypes::Enumeration:
        case CNodeTypes::Typedef:
            return;
        
        // statements
        case CNodeTypes::Block:
            AnalyzeBlock( (BlockNode*)Node );
            return;
        case CNodeTypes::If:
            AnalyzeIf( (IfNode*)Node );
            return;
        case CNodeTypes::While:
            AnalyzeWhile( (WhileNode*)Node );
            return;
        case CNodeTypes::Do:
            AnalyzeDo( (DoNode*)Node );
            return;
        case CNodeTypes::For:
            AnalyzeFor( (ForNode*)Node );
            return;
        case CNodeTypes::Return:
            AnalyzeReturn( (ReturnNode*)Node );
            return;
        case CNodeTypes::Break:
            AnalyzeBreak( (BreakNode*)Node );
            return;
        case CNodeTypes::Continue:
            AnalyzeContinue( (ContinueNode*)Node );
            return;
        case CNodeTypes::Switch:
            AnalyzeSwitch( (SwitchNode*)Node );
            return;
        case CNodeTypes::Case:
            AnalyzeCase( (CaseNode*)Node );
            return;
        case CNodeTypes::Default:
            AnalyzeDefault( (DefaultNode*)Node );
            return;
        case CNodeTypes::Label:
            AnalyzeLabel( (LabelNode*)Node );
            return;
        case CNodeTypes::Goto:
            AnalyzeGoto( (GotoNode*)Node );
            return;
        case CNodeTypes::AssemblyBlock:
            AnalyzeAssemblyBlock( (AssemblyBlockNode*)Node );
            return;
        case CNodeTypes::EmptyStatement:
            // no analysis is needed
            return;
            
        default:
            RaiseFatalError( Node->Location, "unknown AST node type" );
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeExpression( ExpressionNode* Expression, bool MustReturnValue )
{
    // some contructs have optional parts!
    // nothing to do if they are not used
    if( !Expression ) return;
    
    // before any use of an expression after parsing,
    // its return type has to be determined recursively
    Expression->DetermineReturnedType();
    
    // before any emission, all expressions must be checked
    CheckExpression( Expression );
    
    // when requested, ensure the expression returns a value
    if( MustReturnValue )
      if( Expression->ReturnedType->Type() == DataTypes::Void )
        RaiseError( Expression->Location, "This expression does not return a value to be used" );
}


// =============================================================================
//      VIRCON C ANALYZER: ANALYSIS OF DECLARATION NODES
// =============================================================================


void VirconCAnalyzer::AnalyzeVariableList( VariableListNode* VariableList )
{
    // just analyze every listed variable
    for( VariableNode* Variable: VariableList->Variables )
      AnalyzeVariable( Variable );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeVariable( VariableNode* Variable )
{
    // watch for non-instantiable types
    if( Variable->DeclaredType->SizeInWords() == 0 )
    {
        RaiseError( Variable->Location, "variables of type \"" + Variable->DeclaredType->ToString() + "\" cannot be instanced" );
        
        // do not try to analyze anything else
        // since it may cause unforeseen situations
        return;
    }
    
    // analyze variable initialization, if any
    if( Variable->InitialValue )
    {
        // expressions themselves need to be analyzed too
        if( Variable->InitialValue->IsExpression() )
          AnalyzeExpression( (ExpressionNode*)Variable->InitialValue, true );
          
        if( Variable->InitialValue->Type() == CNodeTypes::InitializationList )
          AnalyzeInitializationList( (InitializationListNode*)Variable->InitialValue );
        
        // now we can safely analyze the initialization itself
        AnalyzeInitialization( Variable->Location, Variable->DeclaredType, Variable->InitialValue );
    }
    
    // watch for unused variables
    if( !Variable->IsExtern && !Variable->IsReferenced )
      RaiseWarning( Variable->Location, "variable \"" + Variable->Name + "\" is not used" );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeFunction( FunctionNode* Function )
{
    // analyze return
    if( Function->ReturnType->Type() != DataTypes::Void )
      if( Function->ReturnType->SizeInWords() == 0 )
        RaiseError( Function->Location, "functions cannot return values of type \"" + Function->ReturnType->ToString() + "\"" );
    
    // analyze arguments
    for( auto Argument: Function->Arguments )
    {
        // watch for non-instantiable types
        if( Argument->DeclaredType->SizeInWords() == 0 )
          RaiseError( Argument->Location, "functions cannot receive arguments of type \"" + Argument->DeclaredType->ToString() + "\"" );
        
        // watch for unused arguments
        else if( !Argument->IsReferenced )
        {
            // makes no sense in partial definitions
            // (use flags are only set in the full definition)
            if( Function->HasBody )
              RaiseWarning( Argument->Location, "argument \"" + Argument->Name + "\" is not used" );
        }
    }
    
    // analyze body contents
    if( Function->HasBody )
      for( auto S: Function->Statements )
        AnalyzeCNode( S );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeStructure( StructureNode* Structure )
{
    // partial definitions have nothing to analyze
    if( !Structure->HasBody ) return;
    
    // all members must have instantiable types
    for( MemberNode* Member: Structure->MembersInOrder )
      if( Member->DeclaredType->SizeInWords() == 0 )
        RaiseError( Member->Location, "structures cannot have members of type \"" + Member->DeclaredType->ToString() + "\"" );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeUnion( UnionNode* Union )
{
    // partial definitions have nothing to analyze
    if( !Union->HasBody ) return;
    
    // all members must have instantiable types
    for( MemberNode* Member: Union->Declarations )
      if( Member->DeclaredType->SizeInWords() == 0 )
        RaiseError( Member->Location, "unions cannot have members of type \"" + Member->DeclaredType->ToString() + "\"" );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeEmbeddedFile( EmbeddedFileNode* EmbeddedFile )
{
    // before embedding the file, check that it exists
    ifstream CheckedFile;
    CheckedFile.open( EmbeddedFile->FilePath, ios::binary | ios::ate );
    
    if( !CheckedFile.good() )
    {
        RaiseError( EmbeddedFile->Location, "cannot open embedded file \"" + EmbeddedFile->FilePath + "\"" );
        CheckedFile.close();
        return;
    }
    
    // check that its size actually matches the declared contents
    int ExpectedSize = 4 * EmbeddedFile->Variable->DeclaredType->SizeInWords();
    
    if( CheckedFile.tellg() != ExpectedSize )
      RaiseError( EmbeddedFile->Location, "size of embedded file does not match specified type "
                + EmbeddedFile->Variable->DeclaredType->ToString() + " (" + to_string(ExpectedSize) + " bytes)" );
    
    // we can now close the file
    CheckedFile.close();
    
    // watch for unused variables
    if( !EmbeddedFile->Variable->IsReferenced )
      RaiseWarning( EmbeddedFile->Variable->Location, "variable \"" + EmbeddedFile->Variable->Name + "\" is not used" );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeInitializationList( InitializationListNode* InitializationList )
{
    // just check every contained value
    for( CNode* Value: InitializationList->AssignedValues )
    {
        if( Value->IsExpression() )
          AnalyzeExpression( (ExpressionNode*)Value, true );
          
        // for nested lists we need recursion
        if( Value->Type() == CNodeTypes::InitializationList )
          AnalyzeInitializationList( (InitializationListNode*)Value );
    }
}


// =============================================================================
//      VIRCON C ANALYZER: ANALYSIS OF VARIABLE INITIALIZATIONS
// =============================================================================


// generic initialization analysis function
void VirconCAnalyzer::AnalyzeInitialization( SourceLocation Location, DataType* VariableType, CNode* InitialValue )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Analyze single-value initialization
    if( InitialValue->IsExpression() )
    {
        // for a string, if the variable is an array, 
        // interpret the assignment as a char-by-char assignment
        if( InitialValue->Type() == CNodeTypes::LiteralString
        &&  VariableType->Type() == DataTypes::Array
        &&  TypeIsThisPrimitive( ((ArrayType*)VariableType)->BaseType, PrimitiveTypes::Int ) )
        {
            int ElementsInArray = ((ArrayType*)VariableType)->NumberOfElements;
            string AssignedString = ((LiteralStringNode*)InitialValue)->Value;
            AnalyzeStringInitialization( Location, ElementsInArray, AssignedString );
        }
        
        // otherwise it is just a regular single assignment
        else CheckAssignmentTypes( Location, VariableType, (ExpressionNode*)InitialValue );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Analyze multiple-value initialization
    else if( InitialValue->Type() == CNodeTypes::InitializationList )
    {
        InitializationListNode* ValueList = (InitializationListNode*)InitialValue;
        CNode* FirstValue = ValueList->AssignedValues.front();
        
        // case 1: assign multiple values to array elements
        if( VariableType->Type() == DataTypes::Array )
          AnalyzeArrayInitialization( Location, (ArrayType*)VariableType, ValueList );
        
        // case 2: assign multiple values to structure members
        else if( VariableType->Type() == DataTypes::Structure )
          AnalyzeStructureInitialization( Location, ((StructureType*)VariableType)->GetDeclaration( true ), ValueList );
        
        // case 3: a 1-value list and 1-element variable are actually a single assignment
        else if( ValueList->AssignedValues.size() == 1 && FirstValue->IsExpression() )
          CheckAssignmentTypes( Location, VariableType, (ExpressionNode*)FirstValue );
        
        // other cases are not allowed and we signal an error
        else
          RaiseError( ValueList->Location, "cannot assign multiple values to assign to " + VariableType->ToString() );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Any other nodes cannot be initial values
    else RaiseFatalError( InitialValue->Location, "incorrect initial value" );
}

// -----------------------------------------------------------------------------

// [int array = string] initalizations
void VirconCAnalyzer::AnalyzeStringInitialization( SourceLocation Location, int NumberOfElements, string Characters )
{
    // first, check that the number of values is not greater
    // than the number of possible assignments on the variable
    int NumberOfValues = Characters.size() + 1;  // include null terminator
    
    if( NumberOfValues > NumberOfElements )
      RaiseError( Location, "too many characters to assign to int[" + to_string( NumberOfElements ) + "]" );
    
    // types were already checked to arrive at
    // this case, so no need to do that again
}

// -----------------------------------------------------------------------------

// [array = list] initialization
void VirconCAnalyzer::AnalyzeArrayInitialization( SourceLocation Location, ArrayType* LeftType, InitializationListNode* ValueList )
{
    // first, check that the number of values is not greater
    // than the number of possible assignments on the variable
    int MaxAssignments = LeftType->NumberOfElements;
    int NumberOfValues = ValueList->AssignedValues.size();
    
    if( NumberOfValues > MaxAssignments )
    {
        RaiseError( Location, "too many values to assign to " + LeftType->ToString() );
        return;
    }
    
    // now check types for every value
    for( CNode* Value: ValueList->AssignedValues )
      AnalyzeInitialization( Value->Location, LeftType->BaseType, Value );
}

// -----------------------------------------------------------------------------

// [structure = list] initialization
void VirconCAnalyzer::AnalyzeStructureInitialization( SourceLocation Location, StructureNode* StructureDefinition, InitializationListNode* ValueList )
{
    int MaxAssignments = StructureDefinition->MembersInOrder.size();
    int NumberOfValues = ValueList->AssignedValues.size();
    
    if( NumberOfValues > MaxAssignments )
    {
        RaiseError( Location, "too many values to assign to structure " + StructureDefinition->Name );
        return;
    }
    
    // now check types for every member
    auto MemberIterator = StructureDefinition->MembersInOrder.begin();
    auto ValueIterator = ValueList->AssignedValues.begin();
    
    while( ValueIterator != ValueList->AssignedValues.end() )
    {
        AnalyzeInitialization( (*ValueIterator)->Location, (*MemberIterator)->DeclaredType, *ValueIterator );
        MemberIterator++;
        ValueIterator++;
    }
}


// =============================================================================
//      VIRCON C ANALYZER: ANALYSIS OF STATEMENT NODES
// =============================================================================


void VirconCAnalyzer::AnalyzeBlock( BlockNode* Block )
{
    // analyze body contents
    for( auto S: Block->Statements )
      AnalyzeCNode( S );
    
    // we need to allocate this scope's local
    // variables in its containing function
    Block->AllocateVariablesInStack();
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeIf( IfNode* If )
{
    // analyze the condition
    AnalyzeExpression( If->Condition, true );
    
    // analyze both parts
    AnalyzeCNode( If->TrueStatement );
    AnalyzeCNode( If->FalseStatement );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeWhile( WhileNode* While )
{
    // analyze the condition
    AnalyzeExpression( While->Condition, true );
    
    // analyze loop body
    AnalyzeCNode( While->LoopStatement );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeDo( DoNode* Do )
{
    // analyze the condition
    AnalyzeExpression( Do->Condition, true );
    
    // analyze loop body
    AnalyzeCNode( Do->LoopStatement );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeFor( ForNode* For )
{
    // analyze the condition
    AnalyzeExpression( For->Condition, true );
    
    // analyze loop actions
    AnalyzeCNode( For->InitialAction );
    AnalyzeCNode( For->IterationAction );
    
    // analyze loop body
    AnalyzeCNode( For->LoopStatement );
    
    // we need to allocate this scope's local
    // variables in its containing function
    For->AllocateVariablesInStack();
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeReturn( ReturnNode* Return )
{
    // check that there is a function context
    if( !Return->FunctionContext )
    {
        RaiseError( Return->Location, "function context has not been resolved for \"return\"" );
        return;
    }
    
    // check whether the function actually returns a value
    FunctionNode* Function = Return->FunctionContext;
    
    if( Function->ReturnType->Type() == DataTypes::Void )
    {
        if( Return->ReturnedExpression )
          RaiseError( Return->Location, "functions with return type void cannot return a value" );
    }
    
    else
    {
        if( !Return->ReturnedExpression )
          RaiseError( Return->Location, "this function must return a value" );
        
        // analyze the returned expression
        AnalyzeExpression( Return->ReturnedExpression, true );
        
        // check compatibility of the returned type
        CheckAssignmentTypes( Return->Location, Function->ReturnType, Return->ReturnedExpression );
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeBreak( BreakNode* Break )
{
    // check that there is a loop context
    if( !Break->Context )
    {
        RaiseError( Break->Location, "loop or switch context has not been resolved for \"break\"" );
        return;
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeContinue( ContinueNode* Continue )
{
    // check that there is a loop context
    if( !Continue->LoopContext )
    {
        RaiseError( Continue->Location, "loop context has not been resolved for \"continue\"" );
        return;
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeSwitch( SwitchNode* Switch )
{
    // a switch node is also a block so first analyze it as such
    AnalyzeBlock( Switch );
    
    // analyze the condition expression
    AnalyzeExpression( Switch->Condition, true );
    
    // check compatibility of the condition type
    PrimitiveType IntegerType( PrimitiveTypes::Int );
    CheckAssignmentTypes( Switch->Location, &IntegerType, Switch->Condition );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeCase( CaseNode* Case )
{
    // analyze the condition expression
    AnalyzeExpression( Case->ValueExpression, true );
    
    // (compatibility of type was already analyzed by the parser)
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeDefault( DefaultNode* Default )
{
    // (nothing to analyze)
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeLabel( LabelNode* Label )
{
    // (nothing to analyze)
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeGoto( GotoNode* Goto )
{
    // resolve the target label
    if( Goto->TargetLabel )
      return;
    
    string LabelName = Goto->LabelName;
    auto Pair = Goto->FunctionContext->DeclaredLabels.find( LabelName );
    
    if( Pair == Goto->FunctionContext->DeclaredLabels.end() )
      RaiseError( Goto->Location, string("label \"") + LabelName + "\" has not been declared in this function" );
    
    Goto->TargetLabel = Pair->second;
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeAssemblyBlock( AssemblyBlockNode* AssemblyBlock )
{
    // (nothing to analyze here)
}


// =============================================================================
//      VIRCON C ANALYZER: GLOBAL PROGRAM ANALYSIS
// =============================================================================


void VirconCAnalyzer::AnalyzeGlobalVariables()
{
    for( auto Pair: ProgramAST->DeclaredIdentifiers )
    {
        CNode* TopLevelDefinition = Pair.second;
        
        // discard non-variable definitions
        if( TopLevelDefinition->Type() != CNodeTypes::Variable )
          continue;
        
        VariableNode* Variable = (VariableNode*)TopLevelDefinition;
        
        // if a full definition was declared at some point,
        // it will be the one that remains in the scope
        // (previous partial ones will be replaced)
        if( Variable->IsExtern )
          RaiseError( Variable->Location, string("variable '") + Variable->Name + "' has been declared but not fully defined" );
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeFunctions()
{
    // all functions must be defined at top level scope
    for( auto Pair: ProgramAST->DeclaredIdentifiers )
    {
        CNode* TopLevelDefinition = Pair.second;
        
        // discard non-function definitions
        if( TopLevelDefinition->Type() != CNodeTypes::Function )
          continue;
        
        FunctionNode* Function = (FunctionNode*)TopLevelDefinition;
        
        // if a full definition was declared at some point,
        // it will be the one that remains in the scope
        // (previous partial ones will be replaced)
        if( !Function->HasBody )
          RaiseError( Function->Location, string("function '") + Function->Name + "' has been declared but not fully defined" );
    }
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeMainFunction()
{
    // check that there is a main function
    CNode* Main = ProgramAST->ResolveIdentifier( "main" );
    
    if( !Main || Main->Type() != CNodeTypes::Function )
    {
        RaiseError( ProgramAST->Location, "function \"main\" is not declared" );
        return;
    }
    
    FunctionNode* MainFunction = (FunctionNode*)Main;
    
    // check its correct prototype
    if( MainFunction->Arguments.size() > 0 )
      RaiseError( MainFunction->Location, "in this compiler, function \"main\" cannot have any arguments" );
    
    if( MainFunction->ReturnType->Type() != DataTypes::Void )
      RaiseError( MainFunction->Location, "in this compiler, function \"main\" cannot return any value" );
}

// -----------------------------------------------------------------------------

void VirconCAnalyzer::AnalyzeErrorHandlerFunction()
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // check that there is an error_handler function
    CNode* ErrorHandler = ProgramAST->ResolveIdentifier( "error_handler" );
    
    if( !ErrorHandler || ErrorHandler->Type() != CNodeTypes::Function )
    {
        RaiseError( ProgramAST->Location, "function \"error_handler\" is not declared" );
        return;
    }
    
    FunctionNode* ErrorHandlerFunction = (FunctionNode*)ErrorHandler;
    
    // check its correct prototype
    if( ErrorHandlerFunction->Arguments.size() > 0 )
      RaiseError( ErrorHandlerFunction->Location, "function \"error_handler\" cannot have any arguments" );
    
    if( ErrorHandlerFunction->ReturnType->Type() != DataTypes::Void )
      RaiseError( ErrorHandlerFunction->Location, "function \"error_handler\" cannot return any value" );
}


// =============================================================================
//      VIRCON C ANALYZER: MAIN ANALYSIS FUNCTION
// =============================================================================


void VirconCAnalyzer::Analyze( TopLevelNode& ProgramAST_, bool IsBios )
{
    // restart analysis
    ProgramAST = &ProgramAST_;
    
    // recursively analyze all nodes in top level
    for( CNode* Statement: ProgramAST->Statements )
      AnalyzeCNode( Statement );
    
    // check that all global variables have been fully defined
    AnalyzeGlobalVariables();
    
    // check that all functions have been fully defined
    AnalyzeFunctions();
    
    // at program level, analyze the main function
    AnalyzeMainFunction();
    
    // if this is a BIOS program, we need to have a
    // specific function for handling hardware errors
    if( IsBios )
      AnalyzeErrorHandlerFunction();
}

