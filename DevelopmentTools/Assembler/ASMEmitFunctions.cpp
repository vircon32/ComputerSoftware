// *****************************************************************************
    // include project headers
    #include "ASMEmitFunctions.hpp"
    #include "VirconASMEmitter.hpp"
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GENERIC EMIT FUNCTIONS
// =============================================================================


void EmitInstructionWithoutOperands( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    Emitter.CheckOperands( Node, 0 );
    
    V32Word InstructionWord = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    Emitter.ROM.push_back( InstructionWord );
}

// -----------------------------------------------------------------------------

void EmitJumpInstruction( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operand
    Emitter.CheckOperands( Node, 1 );
    InstructionOperand Operand = Node.Operands[ 0 ];
    
    // operand cannot be a memory address
    if( Operand.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " address cannot be obtained from memory" );
    
    // CASE 1: Address from a register
    if( Operand.Base.Type == BasicValueTypes::CPURegister )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
        return;
    }
    
    // CASE 2: Address as an immediate (integer / label)
    InstructionWord.AsInstruction.UsesImmediate = 1;
    ImmediateWord.AsInteger = Emitter.GetValueAsAddress( Node, Operand.Base );
    Emitter.ROM.push_back( InstructionWord );
    Emitter.ROM.push_back( ImmediateWord );
}

// -----------------------------------------------------------------------------

void EmitConditionalJumpInstruction( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operand
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // operand 1 must be a register
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be a register" );
        
    // operand 2 cannot be a memory address
    if( Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " address cannot be obtained from memory" );
    
    // CASE 1: Address from a register
    if( Operand2.Base.Type == BasicValueTypes::CPURegister )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
        return;
    }
    
    // CASE 2: Address as an immediate (integer / label)
    InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
    InstructionWord.AsInstruction.UsesImmediate = 1;
    ImmediateWord.AsInteger = Emitter.GetValueAsAddress( Node, Operand2.Base );
    Emitter.ROM.push_back( InstructionWord );
    Emitter.ROM.push_back( ImmediateWord );
}

// -----------------------------------------------------------------------------

void EmitInstructionWith1Register( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operand
    Emitter.CheckOperands( Node, 1 );
    InstructionOperand Operand = Node.Operands[ 0 ];
    
    // operand must be a register
    if( Operand.IsMemoryAddress || Operand.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " operand must be a register" );
    
    // emit the instruction
    InstructionWord.AsInstruction.Register1 = (int)Operand.Base.RegisterField;
    Emitter.ROM.push_back( InstructionWord );
}

// -----------------------------------------------------------------------------

void EmitInstructionWith2Registers( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // both operands must be registers
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " operands must both be registers" );
    
    if( Operand2.IsMemoryAddress || Operand2.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " operands must both be registers" );
    
    // emit the instruction
    InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
    InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
    Emitter.ROM.push_back( InstructionWord );
}

// -----------------------------------------------------------------------------

void EmitInstructionWithRegAndInteger( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // operand 1 must be a register
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be a register" );
    
    // operand 2 cannot be a memory address
    if( Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand cannot be from memory" );
    
    // CASE 1: operand 2 is a literal integer
    if( Operand2.Base.Type == BasicValueTypes::LiteralInteger )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsInteger = Operand2.Base.IntegerField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 2: operand 2 is a register
    else if( Operand2.Base.Type == BasicValueTypes::CPURegister )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
    }
    
    else
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must be a register or an integer" );
}

// -----------------------------------------------------------------------------

void EmitInstructionWithRegAndFloat( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // operand 1 must be a register
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be a register" );
    
    // operand 2 cannot be a memory address
    if( Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand cannot be from memory" );
    
    // CASE 1: operand 2 is a literal float
    if( Operand2.Base.Type == BasicValueTypes::LiteralFloat )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsFloat = Operand2.Base.FloatField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 2: operand 2 is a literal integer
    // (it gets converted into a float automatically)
    else if( Operand2.Base.Type == BasicValueTypes::LiteralInteger )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsFloat = Operand2.Base.IntegerField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 3: operand 2 is a register
    else if( Operand2.Base.Type == BasicValueTypes::CPURegister )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
    }
    
    else
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must be a register or a float" );
}


// =============================================================================
//      EMIT FUNCTIONS FOR SPECIFIC INSTRUCTIONS
// =============================================================================


void EmitHLT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithoutOperands( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitWAIT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithoutOperands( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitJMP( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitJumpInstruction( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCALL( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitJumpInstruction( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitRET( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithoutOperands( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitJT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitConditionalJumpInstruction( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitJF( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitConditionalJumpInstruction( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIEQ( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitINE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIGT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIGE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitILT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitILE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFEQ( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFNE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFGT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFGE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFLT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFLE( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitMOV( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    // gather basic information
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // OPERAND VALIDATIONS
    
    // both operands cannot be memory addresses
    if( Operand1.IsMemoryAddress && Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " operands cannot both be from memory" );
    
    // at least 1 operand must be a register
    bool Op1IsRegister = (!Operand1.IsMemoryAddress && Operand1.Base.Type == BasicValueTypes::CPURegister);
    bool Op2IsRegister = (!Operand2.IsMemoryAddress && Operand2.Base.Type == BasicValueTypes::CPURegister);
    
    if( !Op1IsRegister && !Op2IsRegister )
      Emitter.EmitError( Node.Location, OpCodeName + " must have at least 1 register as operand" );
    
    // operand 1 must be a valid destination
    if( !Op1IsRegister && !Operand1.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be either a register or a memory address" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 1: Register <-- Register
    if( Op1IsRegister && Op2IsRegister )
    {
        InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterFromRegister;
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
        return;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 2: Register <-- XXX
    else if( Op1IsRegister )
    {
        InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 2-A: Register <-- Literal
        if( !Operand2.IsMemoryAddress )
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterFromImmediate;
            InstructionWord.AsInstruction.UsesImmediate = 1;
            ImmediateWord = Emitter.GetValueAsImmediate( Node, Operand2.Base );
            
            Emitter.ROM.push_back( InstructionWord );
            Emitter.ROM.push_back( ImmediateWord );
            return;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 2-B: Register <-- Mem[ Register + Literal ]
        else if( Operand2.HasOffset )
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterFromAddressOffset;
            InstructionWord.AsInstruction.UsesImmediate = 1;
            InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
            ImmediateWord.AsInteger = Operand2.Offset.IntegerField;
            
            Emitter.ROM.push_back( InstructionWord );
            Emitter.ROM.push_back( ImmediateWord );
            return;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 2-C: Register <-- Mem[ Register ]
        else if( Operand2.Base.Type == BasicValueTypes::CPURegister )
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterFromRegisterAddress;
            InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
            Emitter.ROM.push_back( InstructionWord );
            return;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 2-D: Register <-- Mem[ Literal ]
        else
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterFromImmediateAddress;
            InstructionWord.AsInstruction.UsesImmediate = 1;
            ImmediateWord.AsInteger = Emitter.GetValueAsAddress( Node, Operand2.Base );
            
            Emitter.ROM.push_back( InstructionWord );
            Emitter.ROM.push_back( ImmediateWord );
            return;
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CASE 3: XXX <-- Register
    else
    {
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 3-A: Mem[ Register + Literal ] <-- Register
        if( Operand1.HasOffset )
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::AddressOffsetFromRegister;
            InstructionWord.AsInstruction.UsesImmediate = 1;
            InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
            ImmediateWord.AsInteger = Operand1.Offset.IntegerField;
            
            Emitter.ROM.push_back( InstructionWord );
            Emitter.ROM.push_back( ImmediateWord );
            return;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 3-B: Mem[ Register ] <-- Register
        else if( Operand1.Base.Type == BasicValueTypes::CPURegister )
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::RegisterAddressFromRegister;
            InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
            Emitter.ROM.push_back( InstructionWord );
            return;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // CASE 3-C: Mem[ Literal ] <-- Register
        else
        {
            InstructionWord.AsInstruction.AddressingMode = (int)AddressingModes::ImmediateAddressFromRegister;
            InstructionWord.AsInstruction.UsesImmediate = 1;
            ImmediateWord.AsInteger = Emitter.GetValueAsAddress( Node, Operand1.Base );
            
            Emitter.ROM.push_back( InstructionWord );
            Emitter.ROM.push_back( ImmediateWord );
            return;
        }
    }
}

// -----------------------------------------------------------------------------

void EmitLEA( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    // gather basic information
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // OPERAND VALIDATIONS
    
    // operand 1 must be a register
    bool Op1IsRegister = (!Operand1.IsMemoryAddress && Operand1.Base.Type == BasicValueTypes::CPURegister);
    
    if( !Op1IsRegister )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be a register" );
    
    // operand 2 must be a memory address
    if( !Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must be a memory address" );
    
    // operand 2 must use a register
    // (it can be [R] or [R+imm], but not [imm])
    if( Operand2.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must use a register as base address" );
    
    // fill common fields in the instruction
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
    InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
    
    // CASE 1: Register <-- [ Register + Literal ]
    if( Operand2.HasOffset )
    {
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsInteger = Operand2.Offset.IntegerField;
        
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
        return;
    }
    
    // CASE 2: Register <-- [ Register ]
    else
    {
        Emitter.ROM.push_back( InstructionWord );
        return;
    }    
}

// -----------------------------------------------------------------------------

void EmitPUSH( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitPOP( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // operand 1 must be a register
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::CPURegister )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand cannot be from memory" );
    
    // operand 2 must be an I/O port
    if( Operand2.IsMemoryAddress || Operand2.Base.Type != BasicValueTypes::IOPort )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must be an I/O port" );
    
    // emit the instruction
    InstructionWord.AsInstruction.Register1 = (int)Operand1.Base.RegisterField;
    InstructionWord.AsInstruction.PortNumber = (int)Operand2.Base.PortField;
    Emitter.ROM.push_back( InstructionWord );
}

// -----------------------------------------------------------------------------

void EmitOUT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    V32Word InstructionWord = {0};
    V32Word ImmediateWord   = {0};
    InstructionWord.AsInstruction.OpCode = (int)Node.OpCode;
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // obtain its operands
    Emitter.CheckOperands( Node, 2 );
    InstructionOperand Operand1 = Node.Operands[ 0 ];
    InstructionOperand Operand2 = Node.Operands[ 1 ];
    
    // operand 1 must be an I/O port
    if( Operand1.IsMemoryAddress || Operand1.Base.Type != BasicValueTypes::IOPort )
      Emitter.EmitError( Node.Location, OpCodeName + " first operand must be an I/O port" );
    
    // operand 2 cannot be a memory address
    if( Operand2.IsMemoryAddress )
      Emitter.EmitError( Node.Location, OpCodeName + " second operand cannot be from memory" );
    
    // CASE 1: operand 2 is a literal integer
    if( Operand2.Base.Type == BasicValueTypes::LiteralInteger )
    {
        InstructionWord.AsInstruction.PortNumber = (int)Operand1.Base.PortField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsInteger = Operand2.Base.IntegerField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 2: operand 2 is a literal float
    else if( Operand2.Base.Type == BasicValueTypes::LiteralFloat )
    {
        InstructionWord.AsInstruction.PortNumber = (int)Operand1.Base.PortField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsFloat = Operand2.Base.FloatField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 3: operand 2 is a predefined port value
    else if( Operand2.Base.Type == BasicValueTypes::IOPortValue )
    {
        InstructionWord.AsInstruction.PortNumber = (int)Operand1.Base.PortField;
        InstructionWord.AsInstruction.UsesImmediate = 1;
        ImmediateWord.AsInteger = (int)Operand2.Base.PortValueField;
        Emitter.ROM.push_back( InstructionWord );
        Emitter.ROM.push_back( ImmediateWord );
    }
    
    // CASE 4: operand 2 is a register
    else if( Operand2.Base.Type == BasicValueTypes::CPURegister )
    {
        InstructionWord.AsInstruction.PortNumber = (int)Operand1.Base.PortField;
        InstructionWord.AsInstruction.Register2 = (int)Operand2.Base.RegisterField;
        Emitter.ROM.push_back( InstructionWord );
    }
    
    else
      Emitter.EmitError( Node.Location, OpCodeName + " second operand must be a register or a literal (port value/integer/float)" );
}

// -----------------------------------------------------------------------------

void EmitMOVS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithoutOperands( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitSETS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithoutOperands( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCMPS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCIF( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCFI( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCIB( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCFB( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitNOT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitAND( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitOR( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitXOR( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitBNOT( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitSHL( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIADD( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitISUB( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIMUL( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIDIV( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIMOD( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitISGN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIABS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIMIN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitIMAX( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndInteger( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFADD( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFSUB( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFMUL( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFDIV( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFMOD( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFSGN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFMIN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFMAX( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWithRegAndFloat( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFABS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitFLR( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitCEIL( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitROUND( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitSIN( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitACOS( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitATAN2( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith2Registers( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitLOG( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith1Register( Emitter, Node );
}

// -----------------------------------------------------------------------------

void EmitPOW( VirconASMEmitter &Emitter, InstructionNode& Node )
{
    EmitInstructionWith2Registers( Emitter, Node );
}
