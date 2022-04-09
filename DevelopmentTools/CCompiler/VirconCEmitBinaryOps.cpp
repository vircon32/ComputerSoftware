// *****************************************************************************
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "Infrastructure.hpp"
    #include "Definitions.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CORRESPONDENCE OF OPERATORS TO CPU INSTRUCTIONS
// =============================================================================


const map< UnaryOperators, string > UnaryOperatorInstructions =
{
    { UnaryOperators::PlusSign,       ""     },
    { UnaryOperators::MinusSign,      "isgn" },
    { UnaryOperators::LogicalNot,     "bnot" },
    { UnaryOperators::BitwiseNot,     "not"  }
};

// -----------------------------------------------------------------------------

const map< BinaryOperators, string > BinaryOperatorInstructions =
{
    { BinaryOperators::Addition,              "iadd" },
    { BinaryOperators::Subtraction,           "isub" },
    { BinaryOperators::Product,               "imul" },
    { BinaryOperators::Division,              "idiv" },
    { BinaryOperators::Modulus,               "mod"  },
    { BinaryOperators::Equal,                 "ieq"  },
    { BinaryOperators::NotEqual,              "ine"  },
    { BinaryOperators::LessThan,              "ilt"  },
    { BinaryOperators::LessOrEqual,           "ile"  },
    { BinaryOperators::GreaterThan,           "igt"  },
    { BinaryOperators::GreaterOrEqual,        "ige"  },
    { BinaryOperators::LogicalOr,             "or"   },
    { BinaryOperators::LogicalAnd,            "and"  },
    { BinaryOperators::BitwiseOr,             "or"   },
    { BinaryOperators::BitwiseAnd,            "and"  },
    { BinaryOperators::BitwiseXor,            "xor"  },
    { BinaryOperators::ShiftLeft,             "shl"  },
    { BinaryOperators::ShiftRight,            "shl"  },
    { BinaryOperators::AdditionAssignment,    "iadd" },
    { BinaryOperators::SubtractionAssignment, "isub" },
    { BinaryOperators::ProductAssignment,     "imul" },
    { BinaryOperators::DivisionAssignment,    "idiv" },
    { BinaryOperators::ModulusAssignment,     "mod"  },
    { BinaryOperators::BitwiseAndAssignment,  "and"  },
    { BinaryOperators::BitwiseOrAssignment,   "or"   },
    { BinaryOperators::BitwiseXorAssignment,  "xor"  },
    { BinaryOperators::ShiftLeftAssignment,   "shl"  },
    { BinaryOperators::ShiftRightAssignment,  "shl"  }
};

// -----------------------------------------------------------------------------

const map< BinaryOperators, string > ReversedBinaryOperatorInstructions =
{
    { BinaryOperators::LessThan,        "igt"  },
    { BinaryOperators::LessOrEqual,     "ige"  },
    { BinaryOperators::GreaterThan,     "ilt"  },
    { BinaryOperators::GreaterOrEqual,  "ile"  }
};

// -----------------------------------------------------------------------------

string InstructionForUnaryOperator( UnaryOperators Which )
{
    // just search in the map
    auto MapPair = UnaryOperatorInstructions.find( Which );
    if( MapPair != UnaryOperatorInstructions.end() )
      return MapPair->second;
    
    return "???";
}

// -----------------------------------------------------------------------------

string InstructionForBinaryOperator( BinaryOperators Which )
{
    // just search in the map
    auto MapPair = BinaryOperatorInstructions.find( Which );
    if( MapPair != BinaryOperatorInstructions.end() )
      return MapPair->second;
    
    return "???";
}

// -----------------------------------------------------------------------------

string InstructionForReversedBinaryOperator( BinaryOperators Which )
{
    // just search in the map
    auto MapPair = ReversedBinaryOperatorInstructions.find( Which );
    if( MapPair != ReversedBinaryOperatorInstructions.end() )
      return MapPair->second;
    
    return "???";
}


// =============================================================================
//      VIRCON C EMITTER: BINARY OPERATIONS (SPECIAL CASES)
// =============================================================================


void VirconCEmitter::EmitAssignment( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // do some common precalculations
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // CASE 1: left operand has a static memory placement
    if( Operation->LeftOperand->HasStaticPlacement() )
    {
        // get placement for left operand
        MemoryPlacement LeftPlacement = Operation->LeftOperand->GetStaticPlacement();
        
        // CASE 1A: Right operand is static
        if( Operation->RightOperand->IsStatic() )
        {
            // ensure that we get the right type
            StaticValue AssignedValue = Operation->RightOperand->GetStaticValue();
            
            // we need to check because it could be "ptr = 0"
            if( Operation->LeftOperand->ReturnedType->Type() == DataTypes::Primitive )
              AssignedValue.ConvertToType( ((PrimitiveType*)Operation->LeftOperand->ReturnedType)->Which );
            
            string ValueName = AssignedValue.ToString();
            
            // we can NOT do the assignment in a single instruction!
            // (movs always need to use a register)
            AssemblyLines.push_back( "mov " + ResultRegisterName + ", " + ValueName );
            AssemblyLines.push_back( "mov " + LeftPlacement.AddressString() + ", " + ResultRegisterName );
        }
        
        // CASE 1B: Calculate the right operand
        // (and place it directly in the result operand)
        else
        {
            EmitDependentExpression( Operation->RightOperand, Registers, ResultRegister );
            
            // when needed, perform type conversion
            // (the non-static case cannot be "ptr = 0")
            DataType* LeftType = Operation->LeftOperand->ReturnedType;
            DataType* RightType = Operation->RightOperand->ReturnedType;
            
            if( LeftType->Type() == DataTypes::Primitive )
              if( RightType->Type() == DataTypes::Primitive )
                EmitTypeConversion( ResultRegister, ((PrimitiveType*)RightType)->Which, ((PrimitiveType*)LeftType)->Which );
            
            // perform the assignment
            AssemblyLines.push_back( "mov " + LeftPlacement.AddressString() + ", " + ResultRegisterName );
        }
    }
    
    // CASE 2: left operand needs to be precalculated
    else
    {
        // use a register for left placement
        int PlacementRegister = Registers.FirstFreeRegister();
        string PlacementRegisterName = "R" + to_string(PlacementRegister);
        EmitExpressionPlacement( Operation->LeftOperand, Registers, PlacementRegister );
        
        // CASE 2A: Right operand is static
        if( Operation->RightOperand->IsStatic() )
        {
            // ensure that we get the right type
            StaticValue AssignedValue = Operation->RightOperand->GetStaticValue();
            
            // we need to check because it could be "ptr = 0"
            if( Operation->LeftOperand->ReturnedType->Type() == DataTypes::Primitive )
              AssignedValue.ConvertToType( ((PrimitiveType*)Operation->LeftOperand->ReturnedType)->Which );
            
            string ValueName = AssignedValue.ToString();
            
            // we can NOT do the assignment in a single instruction!
            // (movs always need to use a register)
            AssemblyLines.push_back( "mov " + ResultRegisterName + ", " + ValueName );
            AssemblyLines.push_back( "mov " + PlacementRegisterName + ", " + ResultRegisterName );
        }
        
        // CASE 2B: Calculate the right operand
        // (and place it directly in the result operand)
        else
        {
            EmitDependentExpression( Operation->RightOperand, Registers, ResultRegister );
            
            // when needed, perform type conversion
            DataType* LeftType = Operation->LeftOperand->ReturnedType;
            DataType* RightType = Operation->RightOperand->ReturnedType;
            
            if( LeftType->Type() == DataTypes::Primitive )
              if( RightType->Type() == DataTypes::Primitive )
                EmitTypeConversion( ResultRegister, ((PrimitiveType*)RightType)->Which, ((PrimitiveType*)LeftType)->Which );
            
            // perform the assignment
            AssemblyLines.push_back( "mov [" + PlacementRegisterName + "], " + ResultRegisterName );
        }
        
        // free used register
        Registers.RegisterUsed[ PlacementRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitLogicalAnd( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // evaluate left operand as a boolean
    EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
    EmitTypeConversion( ResultRegister, Operation->LeftOperand->ReturnedType, Operation->ReturnedType );
    
    // this operation must short-circuit!
    // if false, return that as the result
    string ShortCircuitLabel = "__LogicalAnd_ShortCircuit_" + to_string( Operation->LabelNumber );
    AssemblyLines.push_back( "jf " + ResultRegisterName + ", " + ShortCircuitLabel );
    
    // when not short-circuited, evaluate right operand
    if( Operation->RightOperand->IsStatic() )
    {
        // precalculate static value
        StaticValue RightValue = Operation->RightOperand->GetStaticValue();
        RightValue.ConvertToType( PrimitiveTypes::Bool );
        
        // emit a simplified operation
        AssemblyLines.push_back( "and " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // evaluate right operand as a boolean
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        EmitTypeConversion( RightRegister, Operation->RightOperand->ReturnedType, Operation->ReturnedType );
        
        // now actually apply the logical AND (which, once operands
        // are converted to bool, is done through a bitwise AND)
        AssemblyLines.push_back( "and " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
    
    // by here, the result is already in its register
    EmitLabel( ShortCircuitLabel );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitLogicalOr( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // evaluate left operand as a boolean
    EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
    EmitTypeConversion( ResultRegister, Operation->LeftOperand->ReturnedType, Operation->ReturnedType );
    
    // this operation must short-circuit!
    // if true, return that as the result
    string ShortCircuitLabel = "__LogicalOr_ShortCircuit_" + to_string( Operation->LabelNumber );
    AssemblyLines.push_back( "jt " + ResultRegisterName + ", " + ShortCircuitLabel );
    
    // when not short-circuited, evaluate right operand
    if( Operation->RightOperand->IsStatic() )
    {
        // precalculate static value
        StaticValue RightValue = Operation->RightOperand->GetStaticValue();
        RightValue.ConvertToType( PrimitiveTypes::Bool );
        
        // emit a simplified operation
        AssemblyLines.push_back( "or " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // evaluate right operand as a boolean
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        EmitTypeConversion( RightRegister, Operation->RightOperand->ReturnedType, Operation->ReturnedType );
        
        // now actually apply the logical OR (which, once operands
        // are converted to bool, is done through a bitwise OR)
        AssemblyLines.push_back( "or " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
    
    // by here, the result is already in its register
    EmitLabel( ShortCircuitLabel );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBitShift( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // determine shift direction
    bool IsRightShift = (Operation->Operator == BinaryOperators::ShiftRight);
    
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // determine which of the operands, if any, is static
    bool LeftIsStatic = Operation->LeftOperand->IsStatic();
    bool RightIsStatic = Operation->RightOperand->IsStatic();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // case 1: no static operands
    if( !LeftIsStatic && !RightIsStatic )
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // calculate both operands
        EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        
        // to shift bits right, we will need to change
        // right operand's sign (CPU has SHL but not SHR)
        if( IsRightShift )
          AssemblyLines.push_back( "isgn " + RightRegisterName );
        
        // emit the instruction for the operation itself
        AssemblyLines.push_back( "shl " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // case 2: secondary operand is static
    if( RightIsStatic )
    {
        // precalculate static value
        StaticValue RightValue = Operation->RightOperand->GetStaticValue();
        
        // place dynamic value directly in the result register
        EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
        
        // to shift bits right, we will need to change
        // right operand's sign (CPU has SHL but not SHR)
        if( IsRightShift )
          RightValue.Word.AsInteger = -RightValue.Word.AsInteger;
        
        // emit the instruction for the operation itself
        AssemblyLines.push_back( "shl " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // case 3: main operand is static
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // precalculate static value
        StaticValue LeftValue = Operation->LeftOperand->GetStaticValue();
        
        // place secondary value in the additional register
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        
        // to shift bits right, we will need to change
        // right operand's sign (CPU has SHL but not SHR)
        if( IsRightShift )
          AssemblyLines.push_back( "isgn " + RightRegisterName );
        
        // we need to emit 2 instructions in this case
        AssemblyLines.push_back( "mov "  + ResultRegisterName + ", " + LeftValue.ToString() );
        AssemblyLines.push_back( "shl " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
}


// =============================================================================
//      VIRCON C EMITTER: BINARY OPERATIONS (GENERAL CASES)
// =============================================================================


// any operation that does not require some
// special processing can be emitted the same way
// if no optimization for static values is needed
void VirconCEmitter::EmitUnoptimizedOperation( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // gather type information
    bool LeftIsFloat  = (Operation->LeftOperand->GetReturnType()  == "float");
    bool RightIsFloat = (Operation->RightOperand->GetReturnType() == "float");
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    string OperandsTargetType = (ResultIsFloat? "float" : "int");
    
    // determine the needed instruction for this operation
    string Instruction = InstructionForBinaryOperator( Operation->Operator );
    if( ResultIsFloat ) Instruction[ 0 ] = 'f';
    
    // now emit evaluation for both operands need
    if( !Operation->LeftOperand->IsStatic() && !Operation->RightOperand->IsStatic() )
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // calculate both operands
        EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        
        // perform type conversions if needed
        // - (float + int) promotes int to float
        // - bool values are already taken as int
        EmitTypeConversion( ResultRegister, Operation->LeftOperand->GetReturnType(), OperandsTargetType );
        EmitTypeConversion( RightRegister, Operation->RightOperand->GetReturnType(), OperandsTargetType );
        
        // emit the actual operation
        string ResultRegisterName = "R" + to_string(ResultRegister);
        AssemblyLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

// compound assignments can be reduced to:
// (1) emitting the non-compound operation
// (2) adding a MOV instruction for the assignment
void VirconCEmitter::EmitCompoundAssignment( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // first, forward this to the specialized
    // emit function for the non-compound operation
    switch( Operation->Operator )
    {
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
            EmitSymmetricOperation( Operation, Registers, ResultRegister );
            break;
            
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
            EmitAsymmetricOperation( Operation, Registers, ResultRegister );
            break;
            
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
            EmitBitShift( Operation, Registers, ResultRegister );
            break;
            
        default:
            RaiseFatalError( Operation->Location, "invalid compound assignment operator" );
    }    
    
    // perform type conversion of the value already in the register
    VariableNode* AssignedVariable = Operation->LeftOperand->GetReturnedVariable();
    
    string ProducedType = Operation->RightOperand->GetReturnType();
    string NeededType = AssignedVariable->TypeName;
    EmitTypeConversion( ResultRegister, ProducedType, NeededType );
    
    // now perform the assignment itself
    string VariableAddress = AssignedVariable->AccessAddressString();
    string ResultRegisterName = "R" + to_string( ResultRegister );
    AssemblyLines.push_back( "mov " + VariableAddress + ", " + ResultRegisterName );
}

// -----------------------------------------------------------------------------

// operations that:
// - Operate on numbers (int, float)
// - Can return either numbers or bools
// - Promote ints to floats, take bools as ints
// - ARE commutative
void VirconCEmitter::EmitSymmetricOperation( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // use simplified emission when optimizations are not needed
    bool LeftIsStatic = Operation->LeftOperand->IsStatic();
    bool RightIsStatic = Operation->RightOperand->IsStatic();
    
    if( !LeftIsStatic && !RightIsStatic )
    {
        EmitUnoptimizedOperation( Operation, Registers, ResultRegister );
        return;
    }
    
    // gather type information
    bool LeftIsFloat  = (Operation->LeftOperand->GetReturnType()  == "float");
    bool RightIsFloat = (Operation->RightOperand->GetReturnType() == "float");
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    string OperandsTargetType = (ResultIsFloat? "float" : "int");
    
    // determine the CPU instruction to use
    // for the given operator and types
    string Instruction = InstructionForBinaryOperator( Operation->Operator );
    if( ResultIsFloat ) Instruction[ 0 ] = 'f';
    
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // here we know exactly 1 of the operands is static
    // (we make use of the fact that this operation is commutative)
    ExpressionNode* StaticOperand  = Operation->LeftOperand;
    ExpressionNode* DynamicOperand = Operation->RightOperand;
    
    if( RightIsStatic )
    {
        StaticOperand = Operation->RightOperand;
        DynamicOperand = Operation->LeftOperand;
    }
    
    // precalculate static value
    StaticValue Value = StaticOperand->GetStaticValue();
    
    // place dynamic value directly in the result register
    EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
    
    // perform type conversions if needed
    // (float + int) promotes int to float
    // bool values are already taken as int
    EmitTypeConversion( ResultRegister, DynamicOperand->GetReturnType(), OperandsTargetType );
    Value.ConvertToType( OperandsTargetType );
    
    // emit the instruction for the operation itself
    AssemblyLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value.ToString() );
}

// -----------------------------------------------------------------------------

// operations that:
// - Operate on numbers (int, float)
// - Promote ints to floats, take bools as ints
// - Are NOT commutative
// - CANNOT be reversed for optimization
void VirconCEmitter::EmitAsymmetricOperation( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // use simplified emission when optimizations are not needed
    bool LeftIsStatic = Operation->LeftOperand->IsStatic();
    bool RightIsStatic = Operation->RightOperand->IsStatic();
    
    if( !LeftIsStatic && !RightIsStatic )
    {
        EmitUnoptimizedOperation( Operation, Registers, ResultRegister );
        return;
    }
    
    // gather type information
    bool LeftIsFloat  = (Operation->LeftOperand->GetReturnType()  == "float");
    bool RightIsFloat = (Operation->RightOperand->GetReturnType() == "float");
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    string OperandsTargetType = (ResultIsFloat? "float" : "int");
    
    // determine the CPU instruction to use
    // for the given operator and types
    string Instruction = InstructionForBinaryOperator( Operation->Operator );
    if( ResultIsFloat ) Instruction[ 0 ] = 'f';
    
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // here we know exactly 1 of the operands is static
    
    // case 1: secondary operand is static
    if( RightIsStatic )
    {
        // precalculate static value
        StaticValue RightValue = Operation->RightOperand->GetStaticValue();
        
        // place dynamic value directly in the result register
        EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
        
        // perform type conversions if needed
        // (float + int) promotes int to float
        // bool values are already taken as int
        EmitTypeConversion( ResultRegister, Operation->LeftOperand->GetReturnType(), OperandsTargetType );
        RightValue.ConvertToType( OperandsTargetType );
        
        // emit the instruction for the operation itself
        AssemblyLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // case 2: main operand is static
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // precalculate static value
        StaticValue LeftValue = Operation->LeftOperand->GetStaticValue();
        
        // place secondary value in the additional register
        EmitDependentExpression( Operation->RightOperand, Registers, RightRegister );
        
        // perform type conversions if needed
        // (float + int) promotes int to float
        // bool values are already taken as int
        EmitTypeConversion( RightRegister, Operation->RightOperand->GetReturnType(), OperandsTargetType );
        LeftValue.ConvertToType( OperandsTargetType );
        
        // we need to emit 2 instructions in this case
        AssemblyLines.push_back( "mov "  + ResultRegisterName + ", " + LeftValue.ToString() );
        AssemblyLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
}


// -----------------------------------------------------------------------------

// operations that:
// - Operate on numbers (int, float)
// - Promote ints to floats, take bools as ints
// - Are NOT commutative
// - CAN be reversed for optimization
void VirconCEmitter::EmitReversibleOperation( BinaryOperationNode* Operation, RegisterAllocation& Registers, int ResultRegister )
{
    // use simplified emission when optimizations are not needed
    bool LeftIsStatic = Operation->LeftOperand->IsStatic();
    bool RightIsStatic = Operation->RightOperand->IsStatic();
    
    if( !LeftIsStatic && !RightIsStatic )
    {
        EmitUnoptimizedOperation( Operation, Registers, ResultRegister );
        return;
    }
    
    // gather type information
    bool LeftIsFloat  = (Operation->LeftOperand->GetReturnType()  == "float");
    bool RightIsFloat = (Operation->RightOperand->GetReturnType() == "float");
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    string OperandsTargetType = (ResultIsFloat? "float" : "int");
    
    // determine the CPU instruction to use
    // for the given operator and types
    string Instruction = InstructionForBinaryOperator( Operation->Operator );
    if( ResultIsFloat ) Instruction[ 0 ] = 'f';
    
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // here we know exactly 1 of the operands is static
    
    // case 1: secondary operand is static
    if( RightIsStatic )
    {
        // precalculate static value
        StaticValue RightValue = Operation->RightOperand->GetStaticValue();
        
        // place dynamic value directly in the result register
        EmitDependentExpression( Operation->LeftOperand, Registers, ResultRegister );
        
        // perform type conversions if needed
        // (float + int) promotes int to float
        // bool values are already taken as int
        EmitTypeConversion( ResultRegister, Operation->LeftOperand->GetReturnType(), OperandsTargetType );
        RightValue.ConvertToType( OperandsTargetType );
        
        // emit the instruction for the operation itself
        AssemblyLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // case 2: optimize by reversing the operator
    else  // LeftIsStatic
    {
        // obtain the reversed instruction
        string ReversedInstruction = InstructionForReversedBinaryOperator( Operation->Operator );
        if( ResultIsFloat ) Instruction[ 0 ] = 'f';
        
        // precalculate static value
        StaticValue LeftValue = Operation->LeftOperand->GetStaticValue();
        
        // place dynamic value directly in the result register
        EmitDependentExpression( Operation->RightOperand, Registers, ResultRegister );
        
        // perform type conversions if needed
        // (float + int) promotes int to float
        // bool values are already taken as int
        EmitTypeConversion( ResultRegister, Operation->RightOperand->GetReturnType(), OperandsTargetType );
        LeftValue.ConvertToType( OperandsTargetType );
        
        // emit the instruction for the reversed operation
        // so that we can reverse operand positions
        AssemblyLines.push_back( ReversedInstruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
    }
}
