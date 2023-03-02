// *****************************************************************************
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      EMIT FUNCTIONS FOR INDIVIDUAL UNARY OPERATIONS
// =============================================================================


void VirconCEmitter::EmitPlusSign( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // unary operator '+' does nothing (leave operand as it is)
    EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitMinusSign( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // first evaluate the operand and place it in the desired register
    EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
    
    // we need different instructions for integers and floats
    bool OperandIsFloat = TypeIsFloat( UnaryOperation->Operand->ReturnedType );
    string Instruction = (OperandIsFloat? "fsgn" : "isgn");
    
    // emit the sign change
    string ResultRegisterName = "R" + to_string(ResultRegister);
    ProgramLines.push_back( Instruction + " " + ResultRegisterName );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitPreIncrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // we need different instructions for integers and floats
    bool OperandIsFloat = TypeIsFloat( UnaryOperation->Operand->ReturnedType );
    bool OperandIsPointer = (UnaryOperation->Operand->ReturnedType->Type() == DataTypes::Pointer );
    string Instruction = (OperandIsFloat? "fadd" : "iadd");
    string Value = (OperandIsFloat? "1.0" : "1");
    
    // adjust the value for pointers
    if( OperandIsPointer )
    {
        DataType* PointedType = ((PointerType*)UnaryOperation->Operand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        Value = to_string( PointedSize );
    }
    
    // if the operand has side effects, first evaluate it!
    // if we only take its placement, the side effects are lost
    if( UnaryOperation->Operand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( UnaryOperation->Operand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // convert register to string
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // optimized case: operand with static placement
    if( UnaryOperation->Operand->HasStaticPlacement() )
    {
        MemoryPlacement OperandPlacement = UnaryOperation->Operand->GetStaticPlacement();
        
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + OperandPlacement.AccessAddressString() + "]" );
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + OperandPlacement.AccessAddressString() + "], " + ResultRegisterName );
    }
    
    // otherwise do the full process
    else
    {
        // we need an extra register
        int AddressRegister = Registers.FirstFreeRegister();
        string AddressRegisterName = "R" + to_string(AddressRegister);
        
        // first place operand address in that register
        EmitExpressionPlacement( UnaryOperation->Operand, Registers, AddressRegister );
        
        // now use that address for the whole process
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + AddressRegisterName + "]" );
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + AddressRegisterName + "], " + ResultRegisterName );
        
        // free used register
        Registers.RegisterUsed[ AddressRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitPreDecrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // we need different instructions for integers and floats
    bool OperandIsFloat = TypeIsFloat( UnaryOperation->Operand->ReturnedType );
    bool OperandIsPointer = (UnaryOperation->Operand->ReturnedType->Type() == DataTypes::Pointer );
    string Instruction = (OperandIsFloat? "fsub" : "isub");
    string Value = (OperandIsFloat? "1.0" : "1");
    
    // adjust the value for pointers
    if( OperandIsPointer )
    {
        DataType* PointedType = ((PointerType*)UnaryOperation->Operand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        Value = to_string( PointedSize );
    }
    
    // if the operand has side effects, first evaluate it!
    // if we only take its placement, the side effects are lost
    if( UnaryOperation->Operand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( UnaryOperation->Operand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // convert register to string
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // optimized case: operand with static placement
    if( UnaryOperation->Operand->HasStaticPlacement() )
    {
        MemoryPlacement OperandPlacement = UnaryOperation->Operand->GetStaticPlacement();
        
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + OperandPlacement.AccessAddressString() + "]" );
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + OperandPlacement.AccessAddressString() + "], " + ResultRegisterName );
    }
    
    // otherwise do the full process
    else
    {
        // we need an extra register
        int AddressRegister = Registers.FirstFreeRegister();
        string AddressRegisterName = "R" + to_string(AddressRegister);
        
        // first place operand address in that register
        EmitExpressionPlacement( UnaryOperation->Operand, Registers, AddressRegister );
        
        // now use that address for the whole process
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + AddressRegisterName + "]" );
        ProgramLines.push_back( Instruction + " " + ResultRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + AddressRegisterName + "], " + ResultRegisterName );
        
        // free used register
        Registers.RegisterUsed[ AddressRegister ] = false;
    }
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitPostIncrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // we need different instructions for integers and floats
    bool OperandIsFloat = TypeIsFloat( UnaryOperation->Operand->ReturnedType );
    bool OperandIsPointer = (UnaryOperation->Operand->ReturnedType->Type() == DataTypes::Pointer );
    string Instruction = (OperandIsFloat? "fadd" : "iadd");
    string Value = (OperandIsFloat? "1.0" : "1");
    
    // adjust the value for pointers
    if( OperandIsPointer )
    {
        DataType* PointedType = ((PointerType*)UnaryOperation->Operand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        Value = to_string( PointedSize );
    }
    
    // if the operand has side effects, first evaluate it!
    // if we only take its placement, the side effects are lost
    if( UnaryOperation->Operand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( UnaryOperation->Operand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // convert register to string
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // reserve a register to do the increment
    // (needed since the initial value must be returned)
    int IncrementRegister = Registers.FirstFreeRegister();
    string IncrementRegisterName = "R" + to_string(IncrementRegister);
    
    // optimized case: operand with static placement
    if( UnaryOperation->Operand->HasStaticPlacement() )
    {
        MemoryPlacement OperandPlacement = UnaryOperation->Operand->GetStaticPlacement();
        
        // place initial value in result, and copy it to increment
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + OperandPlacement.AccessAddressString() + "]" );
        ProgramLines.push_back( "mov " + IncrementRegisterName + ", " + ResultRegisterName );
        
        // now do the increment and save it
        ProgramLines.push_back( Instruction + " " + IncrementRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + OperandPlacement.AccessAddressString() + "], " + IncrementRegisterName );
    }
    
    // otherwise do the full process
    else
    {
        // we need an extra register
        int AddressRegister = Registers.FirstFreeRegister();
        string AddressRegisterName = "R" + to_string(AddressRegister);
        
        // first place operand address in that register
        EmitExpressionPlacement( UnaryOperation->Operand, Registers, AddressRegister );
        
        // place initial value in result, and copy it to increment
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + AddressRegisterName + "]" );
        ProgramLines.push_back( "mov " + IncrementRegisterName + ", " + ResultRegisterName );
        
        // now do the increment and save it
        ProgramLines.push_back( Instruction + " " + IncrementRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + AddressRegisterName + "], " + IncrementRegisterName );
        
        // free address register
        Registers.RegisterUsed[ AddressRegister ] = false;
    }
    
    // free final value register
    Registers.RegisterUsed[ IncrementRegister ] = false;
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitPostDecrement( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // we need different instructions for integers and floats
    bool OperandIsFloat = TypeIsFloat( UnaryOperation->Operand->ReturnedType );
    bool OperandIsPointer = (UnaryOperation->Operand->ReturnedType->Type() == DataTypes::Pointer );
    string Instruction = (OperandIsFloat? "fsub" : "isub");
    string Value = (OperandIsFloat? "1.0" : "1");
    
    // adjust the value for pointers
    if( OperandIsPointer )
    {
        DataType* PointedType = ((PointerType*)UnaryOperation->Operand->ReturnedType)->BaseType;
        int PointedSize = PointedType->SizeInWords();
        Value = to_string( PointedSize );
    }
    
    // if the operand has side effects, first evaluate it!
    // if we only take its placement, the side effects are lost
    if( UnaryOperation->Operand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( UnaryOperation->Operand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // convert register to string
    string ResultRegisterName = "R" + to_string(ResultRegister);
    
    // reserve a register to do the decrement
    // (needed since the initial value must be returned)
    int DecrementRegister = Registers.FirstFreeRegister();
    string DecrementRegisterName = "R" + to_string(DecrementRegister);
    
    // optimized case: operand with static placement
    if( UnaryOperation->Operand->HasStaticPlacement() )
    {
        MemoryPlacement OperandPlacement = UnaryOperation->Operand->GetStaticPlacement();
        
        // place initial value in result, and copy it to decrement
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + OperandPlacement.AccessAddressString() + "]" );
        ProgramLines.push_back( "mov " + DecrementRegisterName + ", " + ResultRegisterName );
        
        // now do the decrement and save it
        ProgramLines.push_back( Instruction + " " + DecrementRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + OperandPlacement.AccessAddressString() + "], " + DecrementRegisterName );
    }
    
    // otherwise do the full process
    else
    {
        // we need an extra register
        int AddressRegister = Registers.FirstFreeRegister();
        string AddressRegisterName = "R" + to_string(AddressRegister);
        
        // first place operand address in that register
        EmitExpressionPlacement( UnaryOperation->Operand, Registers, AddressRegister );
        
        // place initial value in result, and copy it to decrement
        ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + AddressRegisterName + "]" );
        ProgramLines.push_back( "mov " + DecrementRegisterName + ", " + ResultRegisterName );
        
        // now do the decrement and save it
        ProgramLines.push_back( Instruction + " " + DecrementRegisterName + ", " + Value );
        ProgramLines.push_back( "mov [" + AddressRegisterName + "], " + DecrementRegisterName );
        
        // free address register
        Registers.RegisterUsed[ AddressRegister ] = false;
    }
    
    // free final value register
    Registers.RegisterUsed[ DecrementRegister ] = false;
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitLogicalNot( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // place operand value in result
    EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
    
    // careful! for a pointer we need to first convert
    // the value to boolean, since NULL is -1 and not 0
    DataType* OperandType = UnaryOperation->Operand->ReturnedType;
    
    if( OperandType->Type() == DataTypes::Pointer )
    {
        PrimitiveType Boolean( PrimitiveTypes::Bool );
        EmitRegisterTypeConversion( ResultRegister, OperandType, &Boolean );
    }
    
    // apply operation
    string ResultRegisterName = "R" + to_string(ResultRegister);
    ProgramLines.push_back( "bnot " + ResultRegisterName );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitBitwiseNot( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // place operand value in result
    EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
    
    // apply operation
    string ResultRegisterName = "R" + to_string(ResultRegister);
    ProgramLines.push_back( "not " + ResultRegisterName );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitReference( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // if the operand has side effects, first evaluate it!
    // if we only take its placement, the sife effects are lost
    if( UnaryOperation->Operand->HasSideEffects() )
    {
        int TempRegister = Registers.FirstFreeRegister();
        EmitDependentExpression( UnaryOperation->Operand, Registers, TempRegister );
        Registers.RegisterUsed[ TempRegister ] = false;
    }
    
    // otherwise, this is the same as emitting the operand placement
    EmitExpressionPlacement( UnaryOperation->Operand, Registers, ResultRegister );
}

// -----------------------------------------------------------------------------

void VirconCEmitter::EmitDereference( UnaryOperationNode* UnaryOperation, RegisterAllocation& Registers, int ResultRegister )
{
    // place operand value in result
    EmitDependentExpression( UnaryOperation->Operand, Registers, ResultRegister );
    
    // obtain value from memory taking previous value as an address
    string ResultRegisterName = "R" + to_string(ResultRegister);
    ProgramLines.push_back( "mov " + ResultRegisterName + ", [" + ResultRegisterName + "]" );
}
