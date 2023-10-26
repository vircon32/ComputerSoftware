// *****************************************************************************
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      VIRCON C EMITTER: INSTANCE HANDLING
// =============================================================================


VirconCEmitter::VirconCEmitter()
{
    ProgramAST = nullptr;
}

// -----------------------------------------------------------------------------

VirconCEmitter::~VirconCEmitter()
{
    // (do nothing)
}


// =============================================================================
//      VIRCON C EMITTER: EMIT FUNCTIONS FOR ABSTRACT NODE TYPES
// =============================================================================


int VirconCEmitter::EmitCNode( CNode* Node )
{
    // some contructs have optional parts!
    // nothing to do if they are not used
    if( !Node ) return 0;
    
    // separate expressions from the rest
    // (they have their own general emitter)
    if( Node->IsExpression() )
      return EmitRootExpression( (ExpressionNode*)Node );
    
    // for non-expression nodes
    switch( Node->Type() )
    {
        // declarations
        case CNodeTypes::VariableList:
            return EmitVariableList( (VariableListNode*)Node );
        case CNodeTypes::Function:
            return EmitFunction( (FunctionNode*)Node );
        case CNodeTypes::EmbeddedFile:
            return EmitEmbeddedFile( (EmbeddedFileNode*)Node );
            
        // for type declarations, no emission
        // is needed (they only affect the AST)
        case CNodeTypes::Structure:
        case CNodeTypes::Union:
        case CNodeTypes::Enumeration:
        case CNodeTypes::Typedef:
            return 0;
            
        // statements
        case CNodeTypes::Block:
            return EmitBlock( (BlockNode*)Node );
        case CNodeTypes::If:
            return EmitIf( (IfNode*)Node );
        case CNodeTypes::While:
            return EmitWhile( (WhileNode*)Node );
        case CNodeTypes::Do:
            return EmitDo( (DoNode*)Node );
        case CNodeTypes::For:
            return EmitFor( (ForNode*)Node );
        case CNodeTypes::Return:
            return EmitReturn( (ReturnNode*)Node );
        case CNodeTypes::Break:
            return EmitBreak( (BreakNode*)Node );
        case CNodeTypes::Continue:
            return EmitContinue( (ContinueNode*)Node );
        case CNodeTypes::Switch:
            return EmitSwitch( (SwitchNode*)Node );
        case CNodeTypes::Case:
            return EmitCase( (CaseNode*)Node );
        case CNodeTypes::Default:
            return EmitDefault( (DefaultNode*)Node );
        case CNodeTypes::Label:
            return EmitLabel( (LabelNode*)Node );
        case CNodeTypes::Goto:
            return EmitGoto( (GotoNode*)Node );
        case CNodeTypes::AssemblyBlock:
            return EmitAssemblyBlock( (AssemblyBlockNode*)Node );
        case CNodeTypes::EmptyStatement:
            // no emission is needed
            return 0;
            
        default:
            RaiseFatalError( Node->Location, "unknown AST node type" );
    }
}

// -----------------------------------------------------------------------------

// this function is called only by the topmost expression in an
// expression tree, so that a new register allocation is started
int VirconCEmitter::EmitRootExpression( ExpressionNode* Expression )
{
    // some contructs have optional parts!
    // nothing to do if they are not used
    if( !Expression ) return 0;
    
    // determine if a register other than R0 should be used
    bool ProtectR0 = (Expression->Type() != CNodeTypes::FunctionCall)
                  && Expression->UsesFunctionCalls();
    
    // now begin evaluation of a new expression tree
    RegisterAllocation Registers( Expression->Location );
    
    // if functions are internally called, R0 may be implicitely used
    // so instead just emit to R1 and move the result to R0 afterwards
    if( ProtectR0 )
    {
        int ResultRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( Expression, Registers, ResultRegister );
        ProgramLines.push_back( "mov R0, R" + to_string(ResultRegister) );
    }
    
    // in other cases we may use R0 directly
    else
      EmitDependentExpression( Expression, Registers, 0 );
    
    return Registers.HighestUsedRegister;
}

// -----------------------------------------------------------------------------

// this is called to emit non-root expressions: emission of its
// parent expression will call this function so that it takes
// into account previously used registers in the tree evaluation 
void VirconCEmitter::EmitDependentExpression( ExpressionNode* Expression, RegisterAllocation& Registers, int ResultRegister )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SPECIAL CASE: Arrays are emitted as their placement (array decay into pointer)
    // This only happens when the array is not used inside other expressions
    if( Expression->ReturnedType->Type() == DataTypes::Array )
    {
        EmitExpressionPlacement( Expression, Registers, ResultRegister );
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 1: fully static expressions are precalculated and placed directly
    // (that is why case 2 does not include SizeOf)
    if( Expression->IsStatic() )
    {
        StaticValue ResultValue = Expression->GetStaticValue();
        string ResultName = ResultValue.ToString();
        
        string ResultRegisterName = "R" + to_string(ResultRegister);
        ProgramLines.push_back( "mov " + ResultRegisterName + ", " + ResultName );
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 2: emit the code needed to evaluate the expression and return it
    switch( Expression->Type() )
    {
        case CNodeTypes::ExpressionAtom:
            EmitExpressionAtom( (ExpressionAtomNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::FunctionCall:
            EmitFunctionCall( (FunctionCallNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::ArrayAccess:
            EmitArrayAccess( (ArrayAccessNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::UnaryOperation:
            EmitUnaryOperation( (UnaryOperationNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::BinaryOperation:
            EmitBinaryOperation( (BinaryOperationNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::EnclosedExpression:
            EmitEnclosedExpression( (EnclosedExpressionNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::MemberAccess:
            EmitMemberAccess( (MemberAccessNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::PointedMemberAccess:
            EmitPointedMemberAccess( (PointedMemberAccessNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::LiteralString:
            EmitLiteralString( (LiteralStringNode*)Expression, Registers, ResultRegister );
            break;
            
        case CNodeTypes::TypeConversion:
            EmitTypeConversion( (TypeConversionNode*)Expression, Registers, ResultRegister );
            break;
            
        default:
            RaiseFatalError( Expression->Location, "invalid expression type" );
    }
}


// =============================================================================
//      VIRCON C EMITTER: NON-NODE EMISSION FUNCTIONS
// =============================================================================


void VirconCEmitter::EmitLabel( const string& LabelName )
{
    // OPTIMIZATION: eliminate jumps to the next line
    // (program will continue to the next line anyway)
    if( !ProgramLines.empty() )
      if( ProgramLines.back() == ("jmp " + LabelName) )
        ProgramLines.pop_back();
    
    // now emit the label normally
    ProgramLines.push_back( LabelName + ":" );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitRegisterTypeConversion( int RegisterNumber, PrimitiveTypes ProducedType, PrimitiveTypes NeededType )
{
    string RegisterName = "R" + to_string( RegisterNumber );
    
    if( NeededType == PrimitiveTypes::Float )
    {
        if( ProducedType != PrimitiveTypes::Float )
          ProgramLines.push_back( "cif " + RegisterName );
    }
    
    else if( NeededType == PrimitiveTypes::Bool )
    {
        if( ProducedType == PrimitiveTypes::Int )
          ProgramLines.push_back( "cib " + RegisterName );
        
        if( ProducedType == PrimitiveTypes::Float )
          ProgramLines.push_back( "cfb " + RegisterName );
    }
    
    else if( NeededType == PrimitiveTypes::Int )
    {
        if( ProducedType == PrimitiveTypes::Float )
          ProgramLines.push_back( "cfi " + RegisterName );
        
        // no need for bool->int conversion since
        // bool is already implemented as an int
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitRegisterTypeConversion( int RegisterNumber, DataType* ProducedType, DataType* NeededType )
{
    // any invalid cases should not appear here
    // (CheckTypeconversion should have discarded them)
    
    // CASE 1: Conversion of a pointer to boolean;
    // since NULL is -1 and not 0 we can't just do an
    // int to bool, and we need to emit a comparison
    if( ProducedType->Type() == DataTypes::Pointer )
      if( NeededType->Type() == DataTypes::Primitive )
        if( ((PrimitiveType*)NeededType)->Which == PrimitiveTypes::Bool )
        {
            string RegisterName = "R" + to_string( RegisterNumber );
            ProgramLines.push_back( "ine " + RegisterName + ", -1" );            
        }
    
    // CASE 2: Conversion of an enumeration to a primitive
    // is the same as with a produced integer
    if( ProducedType->Type() == DataTypes::Enumeration )
      if( NeededType->Type() == DataTypes::Primitive )
        EmitRegisterTypeConversion( RegisterNumber, PrimitiveTypes::Int, ((PrimitiveType*)NeededType)->Which );
    
    // CASE 3: Conversion between primitives
    // only applies if both are primitives
    if( ProducedType->Type() == DataTypes::Primitive )
      if( NeededType->Type() == DataTypes::Primitive )
        EmitRegisterTypeConversion( RegisterNumber, ((PrimitiveType*)ProducedType)->Which, ((PrimitiveType*)NeededType)->Which );
    
    // CASE 4: Conversion of pointers to void*
    // (just a reinterpretation: no actual conversion needed)
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitGlobalScopeFunction()
{   
    // determine the needed stack space for initializations
    int NeededStackSize = ProgramAST->StackSizeForFunctionCalls
                        + ProgramAST->StackSizeForTemporaries;
    
    // (1) function call label
    EmitLabel( "__global_scope_initialization" );
    
    // (2) save caller's stack frame
    ProgramLines.push_back( "push BP" );
    ProgramLines.push_back( "mov BP, SP" );
    
    // (3) allocate space for locals (isub sp, x)
    if( NeededStackSize > 0 )
      ProgramLines.push_back( "isub SP, " + to_string( NeededStackSize ) );
    
    // (4) as body, emit the initialization of all global variables in order
    for( CNode* Statement: ProgramAST->Statements )
      if( Statement->Type() == CNodeTypes::VariableList )
        EmitVariableList( (VariableListNode*)Statement );
    
    // (5) restore the parent's stack frame
    ProgramLines.push_back( "mov SP, BP" );
    ProgramLines.push_back( "pop BP" );
    
    // (6) return to caller
    ProgramLines.push_back( "ret" );
    ProgramLines.push_back( "" );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitProgramStartSection()
{
    // now emit a jump to main functions, that will avoid function code
    ProgramLines.push_back( "; program start section" );
    ProgramLines.push_back( "call __global_scope_initialization" );
    ProgramLines.push_back( "call __function_main" );
    ProgramLines.push_back( "hlt" );
    ProgramLines.push_back( "" );
    
    //  emit names of global variables (but not their initialization)
    ProgramLines.push_back( "; location of global variables" );
    
    for( CNode* Statement: ProgramAST->Statements )
    {
        if( Statement->Type() != CNodeTypes::VariableList )
          continue;
        
        VariableListNode* VariableList = (VariableListNode*)Statement;
        
        for( VariableNode* Variable: VariableList->Variables )
        {
            // do not emit externs, they are partial definitions
            if( Variable->IsExtern )
              continue;
            
            // emit variable name for easier reading
            ProgramLines.push_back( "define global_" + Variable->Name + " " + to_string(Variable->Placement.GlobalAddress) );
        }
    }
    
    ProgramLines.push_back( "" );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitHardwareErrorVector()
{
    // emit this section
    ProgramLines.push_back( "; hardware error vector" );
    ProgramLines.push_back( "call __function_error_handler" );
    ProgramLines.push_back( "wait" );
    ProgramLines.push_back( "hlt" );
    ProgramLines.push_back( "" );
}


// =============================================================================
//      VIRCON C EMITTER: EMISSION FUNCTIONS FOR MEMORY ADDRESSES
// =============================================================================


void VirconCEmitter::EmitStaticPlacement( MemoryPlacement Placement, int ResultRegister )
{
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // LEA instruction needs to use a register as a base, so
    // check the placement to see if MOV must be used instead
    if( Placement.OffsetFromBP != 0 )
      ProgramLines.push_back( "lea " + ResultRegisterName + ", [" + Placement.AccessAddressString() + "]" );
    else
      ProgramLines.push_back( "mov " + ResultRegisterName + ", " + Placement.AccessAddressString() );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitExpressionPlacement( ExpressionNode* Expression, RegisterAllocation& Registers, int ResultRegister )
{
    // check that expression actually has memory placement
    if( !Expression->HasMemoryPlacement() )
      RaiseFatalError( Expression->Location, "cannot emit memory placement when an expression has none" );
    
    // for a static placement, just move it to a register
    if( Expression->HasStaticPlacement() )
    {
        EmitStaticPlacement( Expression->GetStaticPlacement(), ResultRegister );
        return;
    }
    
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // for actual placement computation, select the type of expression
    if( Expression->Type() == CNodeTypes::ExpressionAtom )
    {
        ExpressionAtomNode* Atom = (ExpressionAtomNode*)Expression;
        MemoryPlacement Placement = Atom->ResolvedVariable->Placement;
        
        // The only non-static placements for atom variables happen
        // on embedded files, so there cannot be compound placements.
        // We can just put the variable placement address in the register
        ProgramLines.push_back( "mov " + ResultRegisterName + ", " + Placement.AccessAddressString() );
    }
    
    else if( Expression->Type() == CNodeTypes::ArrayAccess )
    {
        ArrayAccessNode* ArrayAccess = (ArrayAccessNode*)Expression;
        
        // first emit placement for the array:
        // CASE 1: for pointers we need its pointed address, i.e. the pointer value
        if( ArrayAccess->ArrayOperand->ReturnedType->Type() == DataTypes::Pointer )
          EmitDependentExpression( ArrayAccess->ArrayOperand, Registers, ResultRegister );
        
        // CASE 2: for arrays, we do use the placement of the array itself
        else
          EmitExpressionPlacement( ArrayAccess->ArrayOperand, Registers, ResultRegister );
        
        // find out the type of the array elements
        DataType* ElementType = ArrayAccess->ReturnedType;
        int ElementSize = ElementType->SizeInWords();
        
        // now add the offset
        if( ArrayAccess->IndexOperand->IsStatic() )
        {
            // obtain index value
            StaticValue IndexValue = ArrayAccess->IndexOperand->GetStaticValue();
            
            // we only need to do this for non-zero index
            if( IndexValue.Word.AsInteger != 0 )
            {
                // scale the index with element size
                IndexValue.Word.AsInteger *= ElementSize;
                
                // add the resulting offset to form the final placement
                ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IndexValue.ToString() );
            }
        }
        
        else
        {
            // here we need to use an additional register
            int IndexRegister = Registers.FirstFreeRegister();
            string IndexRegisterName = "R" + to_string(IndexRegister);
            
            // emit index of this element in the array
            EmitDependentExpression( ArrayAccess->IndexOperand, Registers, IndexRegister );
            
            // only if element size is not 1, we need to scale the offset
            if( ElementSize != 1 )
              ProgramLines.push_back( "imul " + IndexRegisterName + ", " + to_string(ElementSize) );
            
            // add the resulting offset to form the final placement
            ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IndexRegisterName );
            
            // free the used register
            Registers.RegisterUsed[ IndexRegister ] = false;
        }
    }
    
    else if( Expression->Type() == CNodeTypes::UnaryOperation )
    {
        UnaryOperationNode* UnaryOperation = (UnaryOperationNode*)Expression;
        
        // case 1: pointer dereference
        if( UnaryOperation->Operator == UnaryOperators::Dereference )
        {
            // for pointer arithmetic values obtained on the fly:
            if( !UnaryOperation->Operand->HasMemoryPlacement() )
            {
                EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
            }
            
            // for any other cases, that are based on variables:
            else
            {
                // resulting address is the value pointed by the operand
                EmitExpressionPlacement( UnaryOperation->Operand, Registers, ResultRegister );
                ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + ResultRegisterName + "]" );
            }
        }
        
        // case 2: increment & decrement
        else
        {
            // resulting address is the address of the operand
            EmitExpressionPlacement( UnaryOperation->Operand, Registers, ResultRegister );
        }
    }
    
    else if( Expression->Type() == CNodeTypes::BinaryOperation )
    {
        BinaryOperationNode* BinaryOperation = (BinaryOperationNode*)Expression;
        
        // all cases are assignments, so the resulting
        // address is the address of the left operand
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, ResultRegister );
    }
    
    else if( Expression->Type() == CNodeTypes::EnclosedExpression )
    {
        EnclosedExpressionNode* EnclosedExpression = (EnclosedExpressionNode*)Expression;
        EmitExpressionPlacement( EnclosedExpression->InternalExpression, Registers, ResultRegister );
    }
    
    else if( Expression->Type() == CNodeTypes::MemberAccess )
    {
        MemberAccessNode* MemberAccess = (MemberAccessNode*)Expression;
        
        // first emit placement for the group
        EmitExpressionPlacement( MemberAccess->GroupOperand, Registers, ResultRegister );
        
        // now add the member offset
        int MemberOffset = MemberAccess->ResolvedMember->OffsetInGroup;
        
        if( MemberOffset != 0 )
          ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + to_string( MemberOffset ) );
    }
    
    else if( Expression->Type() == CNodeTypes::PointedMemberAccess )
    {
        PointedMemberAccessNode* PointedMemberAccess = (PointedMemberAccessNode*)Expression;
        
        // first emit the group pointer's value
        // (which is the group's placement)
        EmitDependentExpression( PointedMemberAccess->GroupOperand, Registers, ResultRegister );
        
        // now add the member offset
        int MemberOffset = PointedMemberAccess->ResolvedMember->OffsetInGroup;
        
        if( MemberOffset != 0 )
          ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + to_string( MemberOffset ) );
    }
    
    else
      RaiseFatalError( Expression->Location, "cannot emit memory placement: invalid expression type" );
}


// =============================================================================
//      VIRCON C EMITTER: MAIN EMISSION FUNCTION
// =============================================================================


void VirconCEmitter::Emit( TopLevelNode& ProgramAST_, bool IsBios )
{
    // restart emission
    ProgramAST = &ProgramAST_;
    
    // delete any previous results
    ProgramLines.clear();
    
    // if this is a BIOS program, we need to emit a very
    // specific initial structure for handling hardware errors
    if( IsBios )
      EmitHardwareErrorVector();
    
    // now emit the program start section, which includes:
    // 1) initialization of global variables
    // 2) a call to the main function
    // 3) a halt after main returns
    EmitProgramStartSection();
    EmitGlobalScopeFunction();
    
    // produce assembly code for everything BUT the
    // global variables (already in the start section)
    for( CNode* Statement: ProgramAST->Statements )
      if( Statement->Type() != CNodeTypes::VariableList )
        EmitCNode( Statement );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::SaveAssembly( const std::string& FilePath )
{
    // open output file, in text mode
    ofstream OutputFile;
    OutputFile.open( FilePath, ios_base::out );
    
    if( OutputFile.fail() )
      throw runtime_error( "cannot open output file \"" + FilePath + "\"" );
    
    // write the whole program assembly into the output
    for( string Line: ProgramLines )
    {
        // all lines are indented except
        // for labels and comments
        if( !Line.empty() )
          if( Line[0] != ';' && Line[0] != '_' )
            OutputFile << "  ";
        
        OutputFile << Line << endl;
    }
    
    // write the whole data assembly into the output
    for( string Line: DataLines )
    {
        // all lines are indented except
        // for labels and comments
        if( !Line.empty() )
          if( Line[0] != ';' && Line[0] != '_' )
            OutputFile << "  ";
        
        OutputFile << Line << endl;
    }
    
    // close output
    OutputFile.close();
}
