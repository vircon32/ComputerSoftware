// *****************************************************************************
    // include our headers
    #include "OperandWriters.hpp"
    #include "VirconDisassembler.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GENERIC OPERAND WRITERS
// =============================================================================


string WriteJumpOperands( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // in this case just write the register
    if( !Instruction.UsesImmediate )
    {
        CPURegisters Register1 = (CPURegisters)Instruction.Register1;
        return " " + RegisterToString( Register1 );
    }
    
    // determine if this label has a name
    uint32_t AddressROMIndex = ImmediateValue.AsBinary - InitialROMAddress;
    auto JDN = Disassembler.JumpDestinationNames.find( AddressROMIndex );
    bool HasLabel = (JDN != Disassembler.JumpDestinationNames.end());
    
    // caution: it may exist but be empty
    if( HasLabel )
      HasLabel = (JDN->second != "");
    
    // return the name if it exists, or else the number
    if( HasLabel )
      return " " + JDN->second;
    else
      return " " + Hex( ImmediateValue.AsBinary, 8 );
}

// -----------------------------------------------------------------------------

string WriteConditionalJumpOperands( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    
    // in this case just write the registers
    if( !Instruction.UsesImmediate )
    {
        CPURegisters Register2 = (CPURegisters)Instruction.Register2;
        return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
    }
    
    // determine if this label has a name
    uint32_t AddressROMIndex = ImmediateValue.AsBinary - InitialROMAddress;
    auto JDN = Disassembler.JumpDestinationNames.find( AddressROMIndex );
    bool HasLabel = (JDN != Disassembler.JumpDestinationNames.end());
    
    // return the name if it exists, or else the number
    if( HasLabel )
      return " " + RegisterToString( Register1 ) + ", " + JDN->second;
    else
      return " " + RegisterToString( Register1 ) + ", " + Hex( ImmediateValue.AsBinary, 8 );
}

// -----------------------------------------------------------------------------

string Write1Register( CPUInstruction Instruction )
{
    CPURegisters Register = (CPURegisters)Instruction.Register1;
    return " " + RegisterToString( Register );
}

// -----------------------------------------------------------------------------

string Write2Registers( CPUInstruction Instruction )
{
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
}

// -----------------------------------------------------------------------------

string WriteRegisterAndInteger( CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    
    if( Instruction.UsesImmediate )
      return " " + RegisterToString( Register1 ) + ", " + to_string( ImmediateValue.AsInteger );
    else
      return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
}

// -----------------------------------------------------------------------------

string WriteRegisterAndFloat( CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    
    if( Instruction.UsesImmediate )
      return " " + RegisterToString( Register1 ) + ", " + to_string( ImmediateValue.AsFloat );
    else
      return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
}

// -----------------------------------------------------------------------------

string WriteRegisterAndBinary( CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    
    if( Instruction.UsesImmediate )
      return " " + RegisterToString( Register1 ) + ", " + Hex( ImmediateValue.AsBinary, 8 );
    else
      return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
}


// =============================================================================
//      SPECIALIZED OPERAND WRITERS
// =============================================================================


string WriteHLT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // (no parameters)
    return "";
}

// -----------------------------------------------------------------------------

string WriteWAIT ( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // (no parameters)
    return "";
}

// -----------------------------------------------------------------------------

string WriteJMP( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteJumpOperands( Disassembler, Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteCALL( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteJumpOperands( Disassembler, Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteRET( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // (no parameters)
    return "";
}

// -----------------------------------------------------------------------------

string WriteJT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteConditionalJumpOperands( Disassembler, Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteJF( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteConditionalJumpOperands( Disassembler, Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIEQ( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteINE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIGT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIGE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteILT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteILE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFEQ( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFNE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFGT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFGE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFLT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFLE( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteMOV( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    AddressingModes Mode = (AddressingModes)Instruction.AddressingMode;
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    bool PositiveOffset = (ImmediateValue.AsInteger > 0);
    
    switch( Mode )
    {
        case AddressingModes::RegisterFromImmediate:
            return " " + RegisterToString( Register1 ) + ", " + Hex( ImmediateValue.AsBinary, 8 );
            
        case AddressingModes::RegisterFromRegister:
            return " " + RegisterToString( Register1 ) + ", " + RegisterToString( Register2 );
            
        case AddressingModes::RegisterFromImmediateAddress:
            return " " + RegisterToString( Register1 ) + ", [" + Hex( ImmediateValue.AsBinary, 8 ) + "]";
            
        case AddressingModes::RegisterFromRegisterAddress:
            return " " + RegisterToString( Register1 ) + ", [" + RegisterToString( Register2 ) + "]";
            
        case AddressingModes::RegisterFromAddressOffset:
            return " " + RegisterToString( Register1 ) + ", [" + RegisterToString( Register2 ) + (PositiveOffset? "+" : "") + to_string( ImmediateValue.AsInteger ) + "]";
            
        case AddressingModes::ImmediateAddressFromRegister:
            return " [" + Hex( ImmediateValue.AsBinary, 8 ) + "], " + RegisterToString( Register2 );
            
        case AddressingModes::RegisterAddressFromRegister:
            return " [" + RegisterToString( Register1 ) + "], " + RegisterToString( Register2 );
            
        case AddressingModes::AddressOffsetFromRegister:
            return " [" + RegisterToString( Register1 ) + (PositiveOffset? "+" : "") + to_string( ImmediateValue.AsInteger ) + "], " + RegisterToString( Register2 );
        
        default: return "";
    }
}

// -----------------------------------------------------------------------------

string WriteLEA( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // common data for both cases
    CPURegisters Register1 = (CPURegisters)Instruction.Register1;
    CPURegisters Register2 = (CPURegisters)Instruction.Register2;
    
    string RegisterName1 = RegisterToString( Register1 );
    string RegisterName2 = RegisterToString( Register2 );
    
    // case 1: [R2 + imm]
    if( Instruction.UsesImmediate )
    {
        bool PositiveOffset = (ImmediateValue.AsInteger > 0);
        return " " + RegisterName1 + ", [" + RegisterName2 + (PositiveOffset? "+" : "") + to_string( ImmediateValue.AsInteger ) + "]";
    }
    
    // case 2: [R2]
    else
      return " " + RegisterName1 + ", [" + RegisterName2 + "]";
}

// -----------------------------------------------------------------------------

string WritePUSH( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WritePOP( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteIN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register = (CPURegisters)Instruction.Register1;
    IOPorts Port = (IOPorts)Instruction.PortNumber;
    
    return " " + RegisterToString( Register ) + ", " + PortToString( Port );
}

// -----------------------------------------------------------------------------

string WriteOUT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    CPURegisters Register = (CPURegisters)Instruction.Register2;
    IOPorts Port = (IOPorts)Instruction.PortNumber;
    
    // simple case: a register is used
    if( !Instruction.UsesImmediate )
      return " " + PortToString( Port ) + ", " + RegisterToString( Register );
    
    // for this case we may want to to convert the immediate
    // value to a predefined port value depending on the port
    // (this makes the instruction much easier to interpret)
    if( Port == IOPorts::GPU_Command )
      return " " + PortToString( Port ) + ", " + GPUCommandToString( (IOPortValues)ImmediateValue.AsInteger );
    
    if( Port == IOPorts::SPU_Command )
      return " " + PortToString( Port ) + ", " + SPUCommandToString( (IOPortValues)ImmediateValue.AsInteger );
    
    if( Port == IOPorts::GPU_ActiveBlending )
      return " " + PortToString( Port ) + ", " + GPUBlendingModeToString( (IOPortValues)ImmediateValue.AsInteger );
    
    if( Port == IOPorts::SPU_ChannelState )
      return " " + PortToString( Port ) + ", " + SPUChannelStateToString( (IOPortValues)ImmediateValue.AsInteger );
    
    // for other ports just write the numeric value
    return " " + PortToString( Port ) + ", " + Hex( ImmediateValue.AsBinary, 8 );
}

// -----------------------------------------------------------------------------

string WriteMOVS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // (no parameters)
    return "";
}

// -----------------------------------------------------------------------------

string WriteSETS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    // (no parameters)
    return "";
}

// -----------------------------------------------------------------------------

string WriteCMPS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteCIF( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteCFI( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteCIB( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteCFB( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteNOT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteAND( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndBinary( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteOR( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndBinary( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteXOR( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndBinary( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteBNOT( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteSHL( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIADD( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteISUB( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIMUL( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIDIV( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIMOD( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteISGN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteIABS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteIMIN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteIMAX( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndInteger( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFADD( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFSUB( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFMUL( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFDIV( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFMOD( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFSGN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteFABS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteFMIN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFMAX( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}

// -----------------------------------------------------------------------------

string WriteFLR( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteCEIL( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteROUND( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteSIN( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteACOS( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WriteATAN2( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write2Registers( Instruction );
}

// -----------------------------------------------------------------------------

string WriteLOG( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return Write1Register( Instruction );
}

// -----------------------------------------------------------------------------

string WritePOW( VirconDisassembler& Disassembler, CPUInstruction Instruction, V32::V32Word ImmediateValue )
{
    return WriteRegisterAndFloat( Instruction, ImmediateValue );
}
