// *****************************************************************************
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      EMIT FUNCTIONS FOR SPECIFIC EXPRESSIONS
// =============================================================================


void VirconCEmitter::EmitExpressionAtom( ExpressionAtomNode* ExpressionAtom, RegisterAllocation& Registers, int ResultRegister )
{
    // if this function gets called, the atom is not static
    // (which means it can only be a variable)
    string ResultRegisterName = "R" + to_string(ResultRegister);
    string VariableAddress = ExpressionAtom->ResolvedVariable->Placement.AccessAddressString();
    
    // place the variable value in the register
    ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + VariableAddress + "]" );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitFunctionCall( FunctionCallNode* FunctionCall, RegisterAllocation& Registers, int ResultRegister )
{
    // obtain the called function
    FunctionNode* Function = FunctionCall->ResolvedFunction;
    
    // use a single register for all parameters
    // (but avoid reserving a register if not needed)
    int ParameterRegister = 0;
    
    if( FunctionCall->Parameters.size() > 0 )
      ParameterRegister = Registers.FirstFreeRegister();
    
    string ParameterRegisterName = "R" + to_string( ParameterRegister );
    
    // find the stack frame where this call is allocated
    StackFrameNode* CallingStackFrame = nullptr;
    CNode* CurrentParent = FunctionCall->Parent;
    
    while( CurrentParent )
    {
        if( CurrentParent->HasStackFrame() )
        {
            CallingStackFrame = (StackFrameNode*)CurrentParent;
            break;
        }
        
        CurrentParent = CurrentParent->Parent;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // First pass: precalculate and save any parameters that use
    // function calls and store them in the stack of temporaries
    // (since stack is LIFO, use reverse iteration order here)
    auto ArgumentPositionRev = Function->Arguments.rbegin();
    auto ParameterPositionRev = FunctionCall->Parameters.rbegin();
    
    while( ArgumentPositionRev != Function->Arguments.rend() )
    {
        // discard arguments that do not use calls
        ExpressionNode* Parameter = *ParameterPositionRev;
        VariableNode* Argument = *ArgumentPositionRev;
        
        if( Parameter->UsesFunctionCalls() )
        {
            // emit the evaluation of this parameter
            // (which can now be done independently from this call)
            EmitDependentExpression( Parameter, Registers, ParameterRegister );
            
            // perform type promotion where needed
            DataType* NeededType = Argument->DeclaredType;
            DataType* ProducedType = Parameter->ReturnedType;
            EmitRegisterTypeConversion( ParameterRegister, ProducedType, NeededType );
            
            // save this value in the stack of temporaries
            // (careful! stack allocation starts at [BP-1])
            int FirstTemporaryOffset = CallingStackFrame->StackSizeForVariables + 1;
            int TemporaryOffsetFromBP = FirstTemporaryOffset + Registers.TemporariesStackSize;
            string TemporaryAddress = (TemporaryOffsetFromBP == 0? "[BP]" : "[BP-" + to_string(TemporaryOffsetFromBP) + "]");
            ProgramLines.push_back( "mov " + TemporaryAddress + ", " + ParameterRegisterName );
            
            // mark the used space for this value in the stack of temporaries
            Registers.TemporariesStackSize += 1;
        }
        
        ArgumentPositionRev++;
        ParameterPositionRev++;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Second pass: process other arguments, and use the precalculated ones
    auto ArgumentPosition = Function->Arguments.begin();
    auto ParameterPosition = FunctionCall->Parameters.begin();
    
    while( ArgumentPosition != Function->Arguments.end() )
    {
        ExpressionNode* Parameter = *ParameterPosition;
        VariableNode* Argument = *ArgumentPosition;
        
        if( Parameter->UsesFunctionCalls() )
        {
            // free the used space in the stack of temporaries
            Registers.TemporariesStackSize -= 1;
            
            // load this value from the stack of temporaries
            // (careful! stack allocation starts at [BP-1])
            int FirstTemporaryOffset = CallingStackFrame->StackSizeForVariables + 1;
            int TemporaryOffsetFromBP = FirstTemporaryOffset + Registers.TemporariesStackSize;
            string TemporaryAddress = (TemporaryOffsetFromBP == 0? "[BP]" : "[BP-" + to_string(TemporaryOffsetFromBP) + "]");
            ProgramLines.push_back( "mov " + ParameterRegisterName + ", " + TemporaryAddress );
        }
        
        else
        {
            // emit the evaluation of this parameter
            EmitDependentExpression( Parameter, Registers, ParameterRegister );
            
            // perform type promotion where needed
            DataType* NeededType = Argument->DeclaredType;
            DataType* ProducedType = Parameter->ReturnedType;
            EmitRegisterTypeConversion( ParameterRegister, ProducedType, NeededType );
        }
        
        // place the result as a passed parameter (end of the stack frame)
        string PassingAddress = Argument->Placement.PassingAddressString();
        ProgramLines.push_back( "mov [" + PassingAddress + "], " + ParameterRegisterName );
        
        ArgumentPosition++;
        ParameterPosition++;
    }
    
    // emit the function call itself
    ProgramLines.push_back( "call __function_" + Function->Name );
    
    // By calling convention, calls produce their result in R0.
    // If needed, place the result in the requested register
    if( ResultRegister != 0 )
    {
        string ResultRegisterName = "R" + to_string(ResultRegister);
        ProgramLines.push_back( "mov " + ResultRegisterName + ", R0" );
    }
    
    // release the arguments register
    Registers.RegisterUsed[ ParameterRegister ] = false;
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitArrayAccess( ArrayAccessNode* ArrayAccess, RegisterAllocation& Registers, int ResultRegister )
{
    // do some common precalculations
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // CASE 1: The whole array access has static address
    if( ArrayAccess->HasStaticPlacement() )
    {
        // just obtain the value stored in that address
        MemoryPlacement Placement = ArrayAccess->GetStaticPlacement();
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + Placement.AccessAddressString() + "]" );
        return;
    }
    
    // CASE 2: Index is static
    if( ArrayAccess->IndexOperand->IsStatic() )
    {
        // obtain placement for the resulting array:
        // - for pointers, this emits its stored value;
        // - for arrays, it only emits the array address
        EmitDependentExpression( ArrayAccess->ArrayOperand, Registers, ResultRegister );
        
        // obtain index value
        StaticValue IndexValue = ArrayAccess->IndexOperand->GetStaticValue();
        
        // we only need to do this for non-zero offsets
        if( IndexValue.Word.AsInteger != 0 )
        {
            // find out the type of the array elements
            DataType* ElementType = ArrayAccess->ReturnedType;
            int ElementSize = ElementType->SizeInWords();
            
            // scale the index with element size
            IndexValue.Word.AsInteger *= ElementSize;
            
            // add the index to the array placement
            ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IndexValue.ToString() );
        }
        
        // place target element value in result register
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + ResultRegisterName + "]" );
        return;
    }
        
    // CASE 3: Both array and index need to be determined
    else
    {
        // reserve additional register for the index
        int IndexRegister = Registers.FirstFreeRegister();
        string IndexRegisterName = "R" + to_string(IndexRegister);
        
        // obtain placement for the resulting array:
        // - for pointers, this emits its stored value;
        // - for arrays, it only emits the array address
        EmitDependentExpression( ArrayAccess->ArrayOperand, Registers, ResultRegister );
        
        // emit index, i.e. obtain offset of this element in the array
        EmitDependentExpression( ArrayAccess->IndexOperand, Registers, IndexRegister );
        
        // find out the type of the array elements
        DataType* ElementType = ArrayAccess->ReturnedType;
        int ElementSize = ElementType->SizeInWords();
        
        // only if element size is not 1, we need to scale the offset
        if( ElementSize != 1 )
          ProgramLines.push_back( "imul " + IndexRegisterName + ", " + to_string(ElementSize) );
        
        // compose placement for target element
        ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IndexRegisterName );
        
        // place target element value in result register
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + ResultRegisterName + "]" );
        
        // free the used register
        Registers.RegisterUsed[ IndexRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitUnaryOperation( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // transfer to the specific unary emitter
    switch( UnaryOperation->Operator )
    {
        case UnaryOperators::PlusSign:
            EmitPlusSign( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::MinusSign:
            EmitMinusSign( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::PreIncrement:
            EmitPreIncrement( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::PreDecrement:
            EmitPreDecrement( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::PostIncrement:
            EmitPostIncrement( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::PostDecrement:
            EmitPostDecrement( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::LogicalNot:
            EmitLogicalNot( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::BitwiseNot:
            EmitBitwiseNot( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::Reference:
            EmitReference( UnaryOperation, Registers, ResultRegister );
            break;
        case UnaryOperators::Dereference:
            EmitDereference( UnaryOperation, Registers, ResultRegister );
            break;
            
        default: RaiseFatalError( UnaryOperation->Location, "invalid unary operator" );
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBinaryOperation( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // transfer to the specific binary emitter
    switch( BinaryOperation->Operator )
    {
        case BinaryOperators::Addition:
            EmitAddition( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Subtraction:
            EmitSubtraction( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Division:
            EmitDivision( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Product:
            EmitProduct( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Modulus:
            EmitModulus( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Equal:
            EmitEqual( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::NotEqual:
            EmitNotEqual( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::LessThan:
            EmitLessThan( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::LessOrEqual:
            EmitLessOrEqual( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::GreaterThan:
            EmitGreaterThan( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::GreaterOrEqual:
            EmitGreaterOrEqual( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::LogicalOr:
            EmitLogicalOr( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::LogicalAnd:
            EmitLogicalAnd( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseOr:
            EmitBitwiseOr( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseAnd:
            EmitBitwiseAnd( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseXor:
            EmitBitwiseXor( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ShiftLeft:
            EmitShiftLeft( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ShiftRight:
            EmitShiftRight( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::Assignment:
            EmitAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::AdditionAssignment:
            EmitAdditionAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::SubtractionAssignment:
            EmitSubtractionAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ProductAssignment:
            EmitProductAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::DivisionAssignment:
            EmitDivisionAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ModulusAssignment:
            EmitModulusAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseAndAssignment:
            EmitBitwiseAndAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseOrAssignment:
            EmitBitwiseOrAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::BitwiseXorAssignment:
            EmitBitwiseXorAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ShiftLeftAssignment:
            EmitShiftLeftAssignment( BinaryOperation, Registers, ResultRegister );
            break;
        case BinaryOperators::ShiftRightAssignment:
            EmitShiftRightAssignment( BinaryOperation, Registers, ResultRegister );
            break;
            
        default: RaiseFatalError( BinaryOperation->Location, "invalid binary operator" );
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitEnclosedExpression( EnclosedExpressionNode* EnclosedExpression, RegisterAllocation& Registers, int ResultRegister )
{
    // just emit the internal expression
    EmitDependentExpression( EnclosedExpression->InternalExpression, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitMemberAccess( MemberAccessNode* MemberAccess, RegisterAllocation& Registers, int ResultRegister )
{
    // if the group operand has side effects, first evaluate it!
    // if we only take its placement, the sife effects are lost
    if( MemberAccess->GroupOperand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( MemberAccess->GroupOperand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // precalculate this name
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // CASE 1: For static placement, we can do this in a single line
    if( MemberAccess->HasStaticPlacement() )
    {
        MemoryPlacement Placement = MemberAccess->GetStaticPlacement();
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + Placement.AccessAddressString() + "]" );
    }
    
    // CASE 2: Otherwise do the full process (unoptimized)
    else
    {
        // reserve a register for group
        int GroupRegister = Registers.FirstFreeRegister();
        string GroupRegisterName = "R" + to_string(GroupRegister);
        
        // obtain placement for GroupOperand
        EmitExpressionPlacement( MemberAccess->GroupOperand, Registers, GroupRegister );
        
        // obtain offset of this member in the group
        int MemberOffset = MemberAccess->ResolvedMember->OffsetInGroup;
        
        // compose placement for target member
        if( MemberOffset != 0 )
          ProgramLines.push_back( "iadd " + GroupRegisterName + ", " + to_string(MemberOffset) );
        
        // place member value in result
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + GroupRegisterName + "]" );
        
        // free the used register
        Registers.RegisterUsed[ GroupRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitPointedMemberAccess( PointedMemberAccessNode* PointedMemberAccess, RegisterAllocation& Registers, int ResultRegister )
{
    // reserve a register for group
    int GroupRegister = Registers.FirstFreeRegister();
    string GroupRegisterName = "R" + to_string(GroupRegister);
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // placement for GroupOperand is the pointer value
    EmitDependentExpression( PointedMemberAccess->GroupOperand, Registers, GroupRegister );
    
    // obtain offset of this member in the group
    int MemberOffset = PointedMemberAccess->ResolvedMember->OffsetInGroup;
    
    // compose placement for target member
    if( MemberOffset != 0 )
      ProgramLines.push_back( "iadd " + GroupRegisterName + ", " + to_string(MemberOffset) );
    
    // place member value in result
    ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + GroupRegisterName + "]" );
    
    // free the used register
    Registers.RegisterUsed[ GroupRegister ] = false;
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitLiteralString( LiteralStringNode* LiteralString, RegisterAllocation& Registers, int ResultRegister )
{
    // data section: emit the string label
    DataLines.push_back( LiteralString->NodeLabel() + ":" );
    
    // data section: emit the string itself
    DataLines.push_back( "string \"" + EscapeCString( LiteralString->Value ) + "\"" );
    
    // program section: write string placement to the register
    string ResultRegisterName = "R" + to_string(ResultRegister);
    ProgramLines.push_back( "mov " + ResultRegisterName + ", " + LiteralString->NodeLabel() );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitTypeConversion( TypeConversionNode* TypeConversion, RegisterAllocation& Registers, int ResultRegister )
{
    // first emit the converted expression
    EmitDependentExpression( TypeConversion->ConvertedExpression, Registers, ResultRegister );
    
    // obtain both types
    DataType* OriginalType = TypeConversion->ConvertedExpression->ReturnedType;
    DataType* RequestedType = TypeConversion->RequestedType;
    
    // only for conversions between primitives, emit the
    // needed conversion instruction (other conversions are
    // just the same value reinterpretated as another type)
    if( OriginalType->Type() == DataTypes::Primitive )
      if( RequestedType->Type() == DataTypes::Primitive )
        EmitRegisterTypeConversion( ResultRegister, ((PrimitiveType*)OriginalType)->Which, ((PrimitiveType*)RequestedType)->Which );
}
