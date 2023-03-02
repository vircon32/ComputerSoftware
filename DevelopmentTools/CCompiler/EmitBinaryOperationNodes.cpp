// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      EMIT FUNCTIONS FOR INDIVIDUAL BINARY OPERATIONS
// =============================================================================


// addition is commutative
// it can also do pointer arithmetic
void VirconCEmitter::EmitAddition( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather type information
    bool LeftIsPointer = (BinaryOperation->LeftOperand->ReturnedType->Type() == DataTypes::Pointer);
    bool RightIsPointer = (BinaryOperation->RightOperand->ReturnedType->Type() == DataTypes::Pointer);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 1: Pointer + int
    if( LeftIsPointer || RightIsPointer )
    {
        // tell which is which
        ExpressionNode* PointerOperand = (LeftIsPointer? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* IntegerOperand = (LeftIsPointer? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // we will need the size of the pointed type
        DataType* PointedType = ((PointerType*)PointerOperand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        
        // get the pointer value
        EmitDependentExpression( PointerOperand, Registers, ResultRegister );
        
        // CASE 1.1: Integer operand is static
        if( IntegerOperand->IsStatic() )
        {
            // obtain the integer value
            StaticValue IntegerValue = IntegerOperand->GetStaticValue();
            
            // pointer arithetic uses pointed type as unit
            if( PointedSize != 1 )
              IntegerValue.Word.AsInteger *= PointedSize;
            
            // emit the addition
            ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IntegerValue.ToString() );
            return;
        }
        
        // CASE 1.2: Integer operand has to be emitted
        else
        {
            // reserve an additional register
            int IntegerRegister = Registers.FirstFreeRegister();
            string IntegerRegisterName = "R" + to_string(IntegerRegister);
            
            // place integer value in the additional register
            EmitDependentExpression( IntegerOperand, Registers, IntegerRegister );
            
            // pointer arithetic uses pointed type as unit
            if( PointedSize != 1 )
              ProgramLines.push_back( "imul " + IntegerRegisterName + ", " + to_string(PointedSize) );
            
            // emit the addition
            ProgramLines.push_back( "iadd " + ResultRegisterName + ", " + IntegerRegisterName );
            
            // release the used register
            Registers.RegisterUsed[ IntegerRegister ] = false;
            return;
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 2: primitive + primitive
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    
    // CASE 2.1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // precalculate this for type conversions
        bool DynamicIsFloat = TypeIsFloat( DynamicOperand->ReturnedType );
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !DynamicIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          Value.ConvertToType( PrimitiveTypes::Float );
        
        // emit the addition
        string Instruction = (ResultIsFloat? "fadd" : "iadd");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2.2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the addition
        string Instruction = (ResultIsFloat? "fadd" : "iadd");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// subtraction is not commutative, but reversible
// it can do pointer arithmetic and calculate
// distance between pointers of equal type
void VirconCEmitter::EmitSubtraction( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather type information
    bool LeftIsPointer = (BinaryOperation->LeftOperand->ReturnedType->Type() == DataTypes::Pointer);
    bool RightIsPointer = (BinaryOperation->RightOperand->ReturnedType->Type() == DataTypes::Pointer);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 1: pointer - pointer
    if( LeftIsPointer && RightIsPointer )
    {
        // emit left pointer value into result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve register to emit right pointer value
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right pointer value into reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the subtraction
        ProgramLines.push_back( "isub " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        
        // pointer arithetic uses pointed type as unit
        DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
        int PointedSize = ((PointerType*)LeftType)->BaseType->SizeInWords();
        
        if( PointedSize != 1 )
          ProgramLines.push_back( "idiv " + ResultRegisterName + ", " + to_string(PointedSize) );
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 2: pointer - int (note that "int - pointer" is illegal)
    else if( LeftIsPointer )
    {
        // tell which is which
        ExpressionNode* PointerOperand = BinaryOperation->LeftOperand;
        ExpressionNode* IntegerOperand = BinaryOperation->RightOperand;
        
        // we will need the size of the pointed type
        DataType* PointedType = ((PointerType*)PointerOperand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        
        // get the pointer value
        EmitDependentExpression( PointerOperand, Registers, ResultRegister );
        
        // CASE 1.1: Integer operand is static
        if( IntegerOperand->IsStatic() )
        {
            // obtain the integer value
            StaticValue IntegerValue = IntegerOperand->GetStaticValue();
            
            // pointer arithetic uses pointed type as unit
            if( PointedSize != 1 )
              IntegerValue.Word.AsInteger *= PointedSize;
            
            // emit the subtraction
            ProgramLines.push_back( "isub " + ResultRegisterName + ", " + IntegerValue.ToString() );
            return;
        }
        
        // CASE 1.2: Integer operand has to be emitted
        else
        {
            // reserve an additional register
            int IntegerRegister = Registers.FirstFreeRegister();
            string IntegerRegisterName = "R" + to_string(IntegerRegister);
            
            // place integer value in the additional register
            EmitDependentExpression( IntegerOperand, Registers, IntegerRegister );
            
            // pointer arithetic uses pointed type as unit
            if( PointedSize != 1 )
              ProgramLines.push_back( "imul " + IntegerRegisterName + ", " + to_string(PointedSize) );
            
            // emit the subtraction
            ProgramLines.push_back( "isub " + ResultRegisterName + ", " + IntegerRegisterName );
            
            // release the used register
            Registers.RegisterUsed[ IntegerRegister ] = false;
            return;
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 3: primitive - primitive
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    
    // CASE 3.1: Right operand is static
    // Here we can emit as in an addition
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the subtraction
        string Instruction = (ResultIsFloat? "fsub" : "isub");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 3.2: Left operand is static
    // Here we need to invert "b-a" as "-a+b"
    else if( LeftIsStatic )
    {
        // obtain the static value
        StaticValue LeftValue = BinaryOperation->LeftOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          LeftValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // change sign of dynamic value
        string SignInstruction = (ResultIsFloat? "fsgn" : "isgn");
        ProgramLines.push_back( SignInstruction + " " + ResultRegisterName );
        
        // now emit an addition instead of subtraction
        // (since we have already reorganized the operation)
        string Instruction = (ResultIsFloat? "fadd" : "iadd");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
        return;
    }
    
    // CASE 3.3: No operand is static
    // Again, this is analogous to addition
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the subtraction
        string Instruction = (ResultIsFloat? "fsub" : "isub");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// similar to addition, but in product
// we have no pointer arithmetic
void VirconCEmitter::EmitProduct( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // precalculate this for type conversions
        bool DynamicIsFloat = TypeIsFloat( DynamicOperand->ReturnedType );
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !DynamicIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          Value.ConvertToType( PrimitiveTypes::Float );
        
        // emit the product
        string Instruction = (ResultIsFloat? "fmul" : "imul");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the product
        string Instruction = (ResultIsFloat? "fmul" : "imul");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Not commutative, and while reversible, we will not do
// so to avoid imprecision errors. Thus, unlike subtraction,
// we don't optimize the left-static case.
// Also, we do static checks for divisions by zero
void VirconCEmitter::EmitDivision( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // check for divisions by 0
        if( ResultIsFloat )
        {
            if( RightValue.Word.AsFloat == 0 )
              RaiseFatalError( BinaryOperation->Location, "division by 0" );
        }
        
        else
        {
            if( RightValue.Word.AsInteger == 0 )
              RaiseFatalError( BinaryOperation->Location, "division by 0" );
        }
        
        // emit the division
        string Instruction = (ResultIsFloat? "fdiv" : "idiv");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Right is not static
    // (even it left is, we cannot optimize that)
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the division
        string Instruction = (ResultIsFloat? "fdiv" : "idiv");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Same as division, but there cannot be any floats
// (so type conversions are not needed)
void VirconCEmitter::EmitModulus( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // check for modulus by 0
        if( RightValue.Word.AsInteger == 0 )
          RaiseFatalError( BinaryOperation->Location, "modulus by 0" );
        
        // emit the modulus
        ProgramLines.push_back( "imod " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Right is not static
    // (even it left is, we cannot optimize that)
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the modulus
        ProgramLines.push_back( "imod " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Equality is commutative.
// Pointers are taken as ints
void VirconCEmitter::EmitEqual( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // special case for multi-word operands
    unsigned OperandSize = BinaryOperation->LeftOperand->ReturnedType->SizeInWords();
    
    if( OperandSize > 1 )
    {
        // emit the comparison as a HW memcmp
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, (int)CPURegisters::DestinationRegister );
        EmitExpressionPlacement( BinaryOperation->RightOperand, Registers, (int)CPURegisters::SourceRegister );
        ProgramLines.push_back( "mov CR, " + to_string( OperandSize ) );
        ProgramLines.push_back( "cmps " + ResultRegisterName );
        ProgramLines.push_back( "bnot " + ResultRegisterName );
        return;
    }
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ResultIsFloat = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // precalculate this for type conversions
        bool DynamicIsFloat = TypeIsFloat( DynamicOperand->ReturnedType );
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ResultIsFloat && !DynamicIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ResultIsFloat )
          Value.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ResultIsFloat? "feq" : "ieq");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ResultIsFloat && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ResultIsFloat && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ResultIsFloat? "feq" : "ieq");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to equal
void VirconCEmitter::EmitNotEqual( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // special case for multi-word operands
    unsigned OperandSize = BinaryOperation->LeftOperand->ReturnedType->SizeInWords();
    
    if( OperandSize > 1 )
    {
        // emit the comparison as a HW memcmp
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, (int)CPURegisters::DestinationRegister );
        EmitExpressionPlacement( BinaryOperation->RightOperand, Registers, (int)CPURegisters::SourceRegister );
        ProgramLines.push_back( "mov CR, " + to_string( OperandSize ) );
        ProgramLines.push_back( "cmps " + ResultRegisterName );
        ProgramLines.push_back( "cib " + ResultRegisterName );
        return;
    }
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ThereAreFloats = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // precalculate this for type conversions
        bool DynamicIsFloat = TypeIsFloat( DynamicOperand->ReturnedType );
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !DynamicIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          Value.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fne" : "ine");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fne" : "ine");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// inequality comparisons are not commutative,
// but they can be reversed. Pointers of equal
// type can be compared, and are taken as ints
void VirconCEmitter::EmitLessThan( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ThereAreFloats = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "flt" : "ilt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Left operand is static
    // Here we need to invert "a<b" as "b>a"
    else if( LeftIsStatic )
    {
        // obtain the static value
        StaticValue LeftValue = BinaryOperation->LeftOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          LeftValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // now emit the opposite comparison
        // (since we have already reorganized the operation)
        string Instruction = (ThereAreFloats? "fgt" : "igt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
        return;
    }
    
    // CASE 3: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "flt" : "ilt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to less than
void VirconCEmitter::EmitLessOrEqual( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ThereAreFloats = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fle" : "ile");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Left operand is static
    // Here we need to invert "a<=b" as "b>=a"
    else if( LeftIsStatic )
    {
        // obtain the static value
        StaticValue LeftValue = BinaryOperation->LeftOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          LeftValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // now emit the opposite comparison
        // (since we have already reorganized the operation)
        string Instruction = (ThereAreFloats? "fge" : "ige");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
        return;
    }
    
    // CASE 3: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fle" : "ile");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to less than
void VirconCEmitter::EmitGreaterThan( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ThereAreFloats = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fgt" : "igt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Left operand is static
    // Here we need to invert "a>b" as "b<a"
    else if( LeftIsStatic )
    {
        // obtain the static value
        StaticValue LeftValue = BinaryOperation->LeftOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          LeftValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // now emit the opposite comparison
        // (since we have already reorganized the operation)
        string Instruction = (ThereAreFloats? "flt" : "ilt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
        return;
    }
    
    // CASE 3: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fgt" : "igt");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to less than
void VirconCEmitter::EmitGreaterOrEqual( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // precalculate this for type conversions
    bool LeftIsFloat  = TypeIsFloat( BinaryOperation->LeftOperand->ReturnedType );
    bool RightIsFloat = TypeIsFloat( BinaryOperation->RightOperand->ReturnedType );
    bool ThereAreFloats = (LeftIsFloat || RightIsFloat);
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          RightValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fge" : "ige");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Left operand is static
    // Here we need to invert "a>=b" as "b<=a"
    else if( LeftIsStatic )
    {
        // obtain the static value
        StaticValue LeftValue = BinaryOperation->LeftOperand->GetStaticValue();
        
        // do type conversion for static value
        if( ThereAreFloats )
          LeftValue.ConvertToType( PrimitiveTypes::Float );
        
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // emit type conversion for dynamic value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // now emit the opposite comparison
        // (since we have already reorganized the operation)
        string Instruction = (ThereAreFloats? "fle" : "ile");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + LeftValue.ToString() );
        return;
    }
    
    // CASE 3: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // emit type conversion for left value
        if( ThereAreFloats && !LeftIsFloat )
          EmitRegisterTypeConversion( ResultRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        if( ThereAreFloats && !RightIsFloat )
          EmitRegisterTypeConversion( RightRegister, PrimitiveTypes::Int, PrimitiveTypes::Float );
        
        // emit the comparison
        string Instruction = (ThereAreFloats? "fge" : "ige");
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Boolean operations are commutative, however they must short circuit.
// For that reason we cannot really change their order.
// As for calculation, they are really bitwise operations in which
// their operands are first converted to 0/1 boolean.
// That's why operands can also be pointers
void VirconCEmitter::EmitLogicalOr( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // emit left value to result register
    EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
    
    // emit type conversion for left value
    EmitRegisterTypeConversion( ResultRegister, BinaryOperation->LeftOperand->ReturnedType, BinaryOperation->ReturnedType );
    
    // this operation must short-circuit!
    // if true, return that as the result
    string ShortCircuitLabel = "__LogicalOr_ShortCircuit_" + to_string( BinaryOperation->LabelNumber );
    ProgramLines.push_back( "jt " + ResultRegisterName + ", " + ShortCircuitLabel );
    
    // when not short-circuited, evaluate right operand
    if( BinaryOperation->RightOperand->IsStatic() )
    {
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        RightValue.ConvertToType( PrimitiveTypes::Bool );
        
        // emit the boolean operation
        ProgramLines.push_back( "or " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        EmitRegisterTypeConversion( RightRegister, BinaryOperation->RightOperand->ReturnedType, BinaryOperation->ReturnedType );
        
        // emit the boolean operation
        ProgramLines.push_back( "or " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
    
    // by here, the result is already in its register
    EmitLabel( ShortCircuitLabel );
}

// -----------------------------------------------------------------------------

// analogous to logical or, however the
// short-circuit logic has to be inverted
void VirconCEmitter::EmitLogicalAnd( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // emit left value to result register
    EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
    
    // emit type conversion for left value
    EmitRegisterTypeConversion( ResultRegister, BinaryOperation->LeftOperand->ReturnedType, BinaryOperation->ReturnedType );
    
    // this operation must short-circuit!
    // if false, return that as the result
    string ShortCircuitLabel = "__LogicalAnd_ShortCircuit_" + to_string( BinaryOperation->LabelNumber );
    ProgramLines.push_back( "jf " + ResultRegisterName + ", " + ShortCircuitLabel );
    
    // when not short-circuited, evaluate right operand
    if( BinaryOperation->RightOperand->IsStatic() )
    {
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // do type conversion for static value
        RightValue.ConvertToType( PrimitiveTypes::Bool );
        
        // emit the boolean operation
        ProgramLines.push_back( "and " + ResultRegisterName + ", " + RightValue.ToString() );
    }
    
    else
    {
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit type conversion for right value
        EmitRegisterTypeConversion( RightRegister, BinaryOperation->RightOperand->ReturnedType, BinaryOperation->ReturnedType );
        
        // emit the boolean operation
        ProgramLines.push_back( "and " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
    }
    
    // by here, the result is already in its register
    EmitLabel( ShortCircuitLabel );
}

// -----------------------------------------------------------------------------

// Bitwise operations are commutative.
// Only integral types are allowed, so no type conversions
void VirconCEmitter::EmitBitwiseOr( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // emit the bitwise operation
        ProgramLines.push_back( "or " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the bitwise operation
        ProgramLines.push_back( "or " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to bitwise or
void VirconCEmitter::EmitBitwiseAnd( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // emit the bitwise operation
        ProgramLines.push_back( "and " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the bitwise operation
        ProgramLines.push_back( "and " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// analogous to bitwise or
void VirconCEmitter::EmitBitwiseXor( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool LeftIsStatic = BinaryOperation->LeftOperand->IsStatic();
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: One of the operands is static
    if( LeftIsStatic || RightIsStatic )
    {
        // tell which is which
        ExpressionNode* StaticOperand  = (LeftIsStatic? BinaryOperation->LeftOperand : BinaryOperation->RightOperand);
        ExpressionNode* DynamicOperand = (LeftIsStatic? BinaryOperation->RightOperand : BinaryOperation->LeftOperand);
        
        // emit the dynamic value to result register
        EmitDependentExpression( DynamicOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue Value = StaticOperand->GetStaticValue();
        
        // emit the bitwise operation
        ProgramLines.push_back( "xor " + ResultRegisterName + ", " + Value.ToString() );
        return;
    }
    
    // CASE 2: No operand is static
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the bitwise operation
        ProgramLines.push_back( "xor " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Bit shifts are not commutative nor reversible.
// Only integral types are allowed, so no type conversions
void VirconCEmitter::EmitShiftLeft( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // emit the bit shift
        ProgramLines.push_back( "shl " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Right is not static
    // (even it left is, we cannot optimize that)
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // emit the bit shift
        ProgramLines.push_back( "shl " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Analogous to shift left, but since the CPU
// can only do shift left we change shift sign
void VirconCEmitter::EmitShiftRight( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // convert result register to string for emission
    string ResultRegisterName = "R" + to_string( ResultRegister );
    
    // gather operand information
    bool RightIsStatic = BinaryOperation->RightOperand->IsStatic();
    
    // CASE 1: Right operand is static
    if( RightIsStatic )
    {
        // emit the dynamic value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // obtain the static value
        StaticValue RightValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // to shift bits right, we will need to change
        // right operand's sign (CPU has SHL but not SHR)
        RightValue.Word.AsInteger = -RightValue.Word.AsInteger;
        
        // emit the bit shift
        ProgramLines.push_back( "shl " + ResultRegisterName + ", " + RightValue.ToString() );
        return;
    }
    
    // CASE 2: Right is not static
    // (even it left is, we cannot optimize that)
    else
    {
        // emit left value to result register
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, ResultRegister );
        
        // reserve an additional register
        int RightRegister = Registers.FirstFreeRegister();
        string RightRegisterName = "R" + to_string(RightRegister);
        
        // emit right value to reserved register
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, RightRegister );
        
        // to shift bits right, we will need to change
        // right operand's sign (CPU has SHL but not SHR)
        ProgramLines.push_back( "isgn " + RightRegisterName );
        
        // emit the bit shift
        ProgramLines.push_back( "shl " + ResultRegisterName + ", " + RightRegisterName );
        
        // release the used register
        Registers.RegisterUsed[ RightRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // do some common precalculations
    string ResultRegisterName = "R" + to_string(ResultRegister);
    DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
    DataType* RightType = BinaryOperation->RightOperand->ReturnedType;
    
    // special case for multi-word operands
    int OperandSize = LeftType->SizeInWords();
    
    if( OperandSize > 1 )
    {
        // emit the assignment as a HW memcpy
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, (int)CPURegisters::DestinationRegister );
        EmitExpressionPlacement( BinaryOperation->RightOperand, Registers, (int)CPURegisters::SourceRegister );
        ProgramLines.push_back( "mov CR, " + to_string( OperandSize ) );
        ProgramLines.push_back( "movs" );
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Emit right value
    
    // 1-A: right is static
    if( BinaryOperation->RightOperand->IsStatic() )
    {
        // ensure that we get the right type
        StaticValue AssignedValue = BinaryOperation->RightOperand->GetStaticValue();
        
        // we need to do the check because it could be "ptr = -1",
        // or assigning to an enum type variable (no conversion needed)
        if( BinaryOperation->LeftOperand->ReturnedType->Type() == DataTypes::Primitive )
          AssignedValue.ConvertToType( ((PrimitiveType*)BinaryOperation->LeftOperand->ReturnedType)->Which );
        
        // place result in destination register
        string ValueName = AssignedValue.ToString();
        ProgramLines.push_back( "mov " + ResultRegisterName + ", " + ValueName );
    }
    
    // 1-B: right is not static
    else
    {
        EmitDependentExpression( BinaryOperation->RightOperand, Registers, ResultRegister );
        
        // when needed, perform type conversion
        // (no check: the non-static case cannot be "ptr = -1")
        EmitRegisterTypeConversion( ResultRegister, RightType, LeftType );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Assign to left address
    
    // if the left side has side effects, first evaluate it!
    // if we only take its placement, the sife effects are lost
    if( BinaryOperation->LeftOperand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // 2-A: left address is static
    if( BinaryOperation->LeftOperand->HasStaticPlacement() )
    {
        // get placement for left operand
        MemoryPlacement LeftPlacement = BinaryOperation->LeftOperand->GetStaticPlacement();
        
        // perform assignment
        ProgramLines.push_back( "mov [" + LeftPlacement.AccessAddressString() + "], " + ResultRegisterName );
    }
    
    // 2-B: left address is not static
    else
    {
        // use a register for left placement
        int PlacementRegister = Registers.FirstFreeRegister();
        string PlacementRegisterName = "R" + to_string(PlacementRegister);
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, PlacementRegister );
        
        // perform the assignment
        ProgramLines.push_back( "mov [" + PlacementRegisterName + "], " + ResultRegisterName );
        
        // free used register
        Registers.RegisterUsed[ PlacementRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

// not an operation per se, just a helper function for
// compound assignments to use the previous result
// already present in the register; none of the
// operators using this allow for multi-word operands
void VirconCEmitter::EmitComplementaryAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // do some common precalculations
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // if the left side has side effects, first evaluate it!
    // if we only take its placement, the sife effects are lost
    if( BinaryOperation->LeftOperand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( BinaryOperation->LeftOperand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // CASE 1: left operand has a static address
    if( BinaryOperation->LeftOperand->HasStaticPlacement() )
    {
        // perform assignment to the static placement
        MemoryPlacement LeftPlacement = BinaryOperation->LeftOperand->GetStaticPlacement();
        ProgramLines.push_back( "mov [" + LeftPlacement.AccessAddressString() + "], " + ResultRegisterName );
        return;
    }
    
    // CASE 2: general case (unoptimized)
    else
    {
        // we need an extra register
        int AddressRegister = Registers.FirstFreeRegister();
        string AddressRegisterName = "R" + to_string(AddressRegister);
        
        // perform the assignment using the reserved register
        EmitExpressionPlacement( BinaryOperation->LeftOperand, Registers, AddressRegister );
        ProgramLines.push_back( "mov [" + AddressRegisterName + "], " + ResultRegisterName );
        
        // free used register
        Registers.RegisterUsed[ AddressRegister ] = false;
        return;
    }
}

// -----------------------------------------------------------------------------

// Unoptimized, this could be done as addition + assignment.
// This is actually optimal when variable has static address.
// We can optimize for non-static addresses by avoiding the
// recomputation of the address, but since it would mean lots
// of additional code we will not do it for now
void VirconCEmitter::EmitAdditionAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitAddition( BinaryOperation, Registers, ResultRegister );
    
    // however we may have to convert type before assignment,
    // if we have situations such as int += float; this only
    // applies in this case and not float += int, where the
    // conversion is already applied by doing type promotion
    DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
    DataType* RightType = BinaryOperation->RightOperand->ReturnedType;
    
    if( !TypeIsFloat( LeftType ) && TypeIsFloat( RightType ) )
      EmitRegisterTypeConversion( ResultRegister, RightType, LeftType );
    
    // now we can safely assign
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitSubtractionAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitSubtraction( BinaryOperation, Registers, ResultRegister );
    
    // however we may have to convert type before assignment,
    // if we have situations such as int += float; this only
    // applies in this case and not float += int, where the
    // conversion is already applied by doing type promotion
    DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
    DataType* RightType = BinaryOperation->RightOperand->ReturnedType;
    
    if( !TypeIsFloat( LeftType ) && TypeIsFloat( RightType ) )
      EmitRegisterTypeConversion( ResultRegister, RightType, LeftType );
    
    // now we can safely assign
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitProductAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitProduct( BinaryOperation, Registers, ResultRegister );
    
    // however we may have to convert type before assignment,
    // if we have situations such as int += float; this only
    // applies in this case and not float += int, where the
    // conversion is already applied by doing type promotion
    DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
    DataType* RightType = BinaryOperation->RightOperand->ReturnedType;
    
    if( !TypeIsFloat( LeftType ) && TypeIsFloat( RightType ) )
      EmitRegisterTypeConversion( ResultRegister, RightType, LeftType );
    
    // now we can safely assign
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitDivisionAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitDivision( BinaryOperation, Registers, ResultRegister );
    
    // however we may have to convert type before assignment,
    // if we have situations such as int += float; this only
    // applies in this case and not float += int, where the
    // conversion is already applied by doing type promotion
    DataType* LeftType = BinaryOperation->LeftOperand->ReturnedType;
    DataType* RightType = BinaryOperation->RightOperand->ReturnedType;
    
    if( !TypeIsFloat( LeftType ) && TypeIsFloat( RightType ) )
      EmitRegisterTypeConversion( ResultRegister, RightType, LeftType );
    
    // now we can safely assign
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitModulusAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitModulus( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBitwiseAndAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitBitwiseAnd( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBitwiseOrAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitBitwiseOr( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBitwiseXorAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitBitwiseXor( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitShiftLeftAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitShiftLeft( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitShiftRightAssignment( BinaryOperationNode* BinaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // emit the non-compound operation + a complementary assigment
    EmitShiftRight( BinaryOperation, Registers, ResultRegister );
    EmitComplementaryAssignment( BinaryOperation, Registers, ResultRegister );
}
