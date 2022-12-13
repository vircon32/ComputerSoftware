// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Definitions.hpp"
    
    // include project headers
    #include "VirconCPU.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Mathematics
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


inline void Push( VirconCPU& CPU, VirconWord Value )
{
    // first decrement
    int32_t* SP = &CPU.StackPointer.AsInteger;
    (*SP)--;
    
    // check for stack overflow
    if( *SP < Constants::RAMFirstAddress )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::StackOverflow );
        return;
    }
    
    // and then store the value
    CPU.MemoryBus->WriteAddress( *SP, Value );
}

// -----------------------------------------------------------------------------

inline void Pop( VirconCPU& CPU, VirconWord& Register )
{
    // first read the value
    int32_t* SP = &CPU.StackPointer.AsInteger;
    
    if( !CPU.MemoryBus->ReadAddress( *SP, Register ) )
      return;
    
    // and then increment
    (*SP)++;
    
    // check for stack underflow
    if( *SP >= (Constants::RAMFirstAddress + Constants::RAMSize) )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::StackUnderflow );
        return;
    }
}


// =============================================================================
//      INSTRUCTION PROCESSORS
// =============================================================================


void ProcessHLT( VirconCPU& CPU, CPUInstruction Instruction )
{
    CPU.Halted = true;
    cout << "CPU halted" << endl;
}

// -----------------------------------------------------------------------------

void ProcessWAIT( VirconCPU& CPU, CPUInstruction Instruction )
{
    CPU.Waiting = true;
}

// -----------------------------------------------------------------------------

void ProcessJMP( VirconCPU& CPU, CPUInstruction Instruction )
{
    if( Instruction.UsesImmediate )
      CPU.InstructionPointer = CPU.ImmediateValue;
    else
      CPU.InstructionPointer = CPU.Registers[ Instruction.Register1 ];
}

// -----------------------------------------------------------------------------

void ProcessCALL( VirconCPU& CPU, CPUInstruction Instruction )
{
    // first push the program counter
    Push( CPU, CPU.InstructionPointer );
    
    // then implement a jump
    if( Instruction.UsesImmediate )
      CPU.InstructionPointer = CPU.ImmediateValue;
    else
      CPU.InstructionPointer = CPU.Registers[ Instruction.Register1 ];
}

// -----------------------------------------------------------------------------

void ProcessRET( VirconCPU& CPU, CPUInstruction Instruction )
{
    // pop the program counter
    Pop( CPU, CPU.InstructionPointer );
}

// -----------------------------------------------------------------------------

void ProcessJT( VirconCPU& CPU, CPUInstruction Instruction )
{
    // check condition
    uint32_t ConditionValue = CPU.Registers[ Instruction.Register1 ].AsBinary;
    if( !ConditionValue ) return;
    
    // perform the jump
    if( Instruction.UsesImmediate )
      CPU.InstructionPointer = CPU.ImmediateValue;
    else
      CPU.InstructionPointer = CPU.Registers[ Instruction.Register2 ];
}

// -----------------------------------------------------------------------------

void ProcessJF( VirconCPU& CPU, CPUInstruction Instruction )
{
    // check condition
    uint32_t ConditionValue = CPU.Registers[ Instruction.Register1 ].AsBinary;
    if( ConditionValue ) return;
    
    // perform the jump
    if( Instruction.UsesImmediate )
      CPU.InstructionPointer = CPU.ImmediateValue;
    else
      CPU.InstructionPointer = CPU.Registers[ Instruction.Register2 ];
}

// -----------------------------------------------------------------------------

void ProcessIEQ( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger == Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessINE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger != Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessIGT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger > Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessIGE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger >= Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessILT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger < Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessILE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsInteger <= Value.AsInteger);
}

// -----------------------------------------------------------------------------

void ProcessFEQ( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat == Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessFNE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat != Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessFGT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat > Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessFGE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat >= Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessFLT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat < Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessFLE( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord Value;
    
    if( Instruction.UsesImmediate )
      Value = CPU.ImmediateValue;
    else
      Value = CPU.Registers[ Instruction.Register2 ];
    
    Register1->AsBinary = (Register1->AsFloat <= Value.AsFloat);
}

// -----------------------------------------------------------------------------

void ProcessMOV( VirconCPU& CPU, CPUInstruction Instruction )
{
    // this is just a dummy function: MOV is not processed here;
    // however, removing it might hinder optimization for for the instruction switch
}

// -----------------------------------------------------------------------------

void ProcessLEA( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsInteger = Register2->AsInteger + CPU.ImmediateValue.AsInteger;
    
    else
      Register1->AsInteger = Register2->AsInteger;
}

// -----------------------------------------------------------------------------

void ProcessPUSH( VirconCPU& CPU, CPUInstruction Instruction )
{
    Push( CPU, CPU.Registers[ Instruction.Register1 ] );
}

// -----------------------------------------------------------------------------

void ProcessPOP( VirconCPU& CPU, CPUInstruction Instruction )
{
    Pop( CPU, CPU.Registers[ Instruction.Register1 ] );
}

// -----------------------------------------------------------------------------

void ProcessIN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    CPU.ControlBus->ReadPort( Instruction.PortNumber, *DestinationRegister );
}

// -----------------------------------------------------------------------------

void ProcessOUT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
    
    if( Instruction.UsesImmediate )
      CPU.ControlBus->WritePort( Instruction.PortNumber, CPU.ImmediateValue );
    else
      CPU.ControlBus->WritePort( Instruction.PortNumber, *SourceRegister );
}

// -----------------------------------------------------------------------------

void ProcessMOVS( VirconCPU& CPU, CPUInstruction Instruction )
{
    // move 1 word as in a supposed MOV [DR], [SR]
    VirconWord Value;
    
    if( !CPU.MemoryBus->ReadAddress( CPU.SourceRegister.AsInteger, Value ) )
      return;
    
    if( !CPU.MemoryBus->WriteAddress( CPU.DestinationRegister.AsInteger, Value ) )
      return;
    
    // increase DR and SR by 1
    CPU.SourceRegister.AsInteger++;
    CPU.DestinationRegister.AsInteger++;
    
    // decrease counter down to 0
    int32_t& Counter = CPU.CountRegister.AsInteger;
    
    if( Counter > 0 )
      Counter--;
    
    // restore PC if count not finished
    if( Counter > 0 )
      CPU.InstructionPointer.AsInteger--;
}

// -----------------------------------------------------------------------------

void ProcessSETS( VirconCPU& CPU, CPUInstruction Instruction )
{
    // set 1 word as in a MOV [DR], SR
    if( !CPU.MemoryBus->WriteAddress( CPU.DestinationRegister.AsInteger, CPU.SourceRegister ) )
      return;
    
    // increase DR by 1
    CPU.DestinationRegister.AsInteger++;
    
    // decrease counter down to 0
    int32_t& Counter = CPU.CountRegister.AsInteger;
    
    if( Counter > 0 )
      Counter--;
    
    // restore PC if count not finished
    if( Counter > 0 )
      CPU.InstructionPointer.AsInteger--;
}

// -----------------------------------------------------------------------------

// the result register will be:
// - positive if string at [DR] comes after string at [SR]
// - negative if string at [DR] comes before string at [SR]
// - zero if both strings are equal
void ProcessCMPS( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* ResultRegister = &CPU.Registers[ Instruction.Register1 ];
    
    // subtract 1 word as in a supposed ResultRegister = [DR] - [SR]
    VirconWord SRValue;
    
    if( !CPU.MemoryBus->ReadAddress( CPU.DestinationRegister.AsInteger, *ResultRegister ) )
      return;
    
    if( !CPU.MemoryBus->ReadAddress( CPU.SourceRegister.AsInteger, SRValue ) )
      return;
    
    ResultRegister->AsInteger -= SRValue.AsInteger;
    
    // if non-zero, comparison has ended
    if( ResultRegister->AsInteger != 0 )
      return;
    
    // increase DR and SR by 1
    CPU.SourceRegister.AsInteger++;
    CPU.DestinationRegister.AsInteger++;
    
    // decrease counter down to 0
    int32_t& Counter = CPU.CountRegister.AsInteger;
    
    if( Counter > 0 )
      Counter--;
    
    // restore PC if count not finished
    if( Counter > 0 )
      CPU.InstructionPointer.AsInteger--;
}

// -----------------------------------------------------------------------------

void ProcessCIF( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsFloat = (float)Register->AsInteger;
}

// -----------------------------------------------------------------------------

void ProcessCFI( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsInteger = (int32_t)Register->AsFloat;
}

// -----------------------------------------------------------------------------

void ProcessCIB( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsInteger = (bool)Register->AsInteger;
}

// -----------------------------------------------------------------------------

void ProcessCFB( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsInteger = (bool)Register->AsFloat;
}

// -----------------------------------------------------------------------------

void ProcessNOT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsBinary = ~Register->AsBinary;
}

// -----------------------------------------------------------------------------

void ProcessAND( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsBinary &= CPU.ImmediateValue.AsBinary;
    else
      Register1->AsBinary &= CPU.Registers[ Instruction.Register2 ].AsBinary;
}

// -----------------------------------------------------------------------------

void ProcessOR( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsBinary |= CPU.ImmediateValue.AsBinary;
    else
      Register1->AsBinary |= CPU.Registers[ Instruction.Register2 ].AsBinary;
}

// -----------------------------------------------------------------------------

void ProcessXOR( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsBinary ^= CPU.ImmediateValue.AsBinary;
    else
      Register1->AsBinary ^= CPU.Registers[ Instruction.Register2 ].AsBinary;
}

// -----------------------------------------------------------------------------

void ProcessBNOT( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
    Register->AsBinary = (Register->AsBinary? 0 : 1);
}

// -----------------------------------------------------------------------------

void ProcessSHL( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    int32_t ShiftAmount;
    
    if( Instruction.UsesImmediate )
      ShiftAmount = CPU.ImmediateValue.AsInteger;
    else
      ShiftAmount = CPU.Registers[ Instruction.Register2 ].AsInteger;
    
    // allow negative shifts
    if( ShiftAmount > 0 )
      Register1->AsBinary <<= ShiftAmount;
    else
      Register1->AsBinary >>= -ShiftAmount;
}

// -----------------------------------------------------------------------------

void ProcessIADD( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsInteger += CPU.ImmediateValue.AsInteger;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsInteger += SourceRegister->AsInteger;
    }
}

// -----------------------------------------------------------------------------

void ProcessISUB( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsInteger -= CPU.ImmediateValue.AsInteger;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsInteger -= SourceRegister->AsInteger;
    }
}

// -----------------------------------------------------------------------------

void ProcessIMUL( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsInteger *= CPU.ImmediateValue.AsInteger;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsInteger *= SourceRegister->AsInteger;
    }
}

// -----------------------------------------------------------------------------

void ProcessIDIV( VirconCPU& CPU, CPUInstruction Instruction )
{
    // choose the requested divisor
    int32_t Divisor = 1;
    
    if( Instruction.UsesImmediate )
      Divisor = CPU.ImmediateValue.AsInteger;
    else
      Divisor = CPU.Registers[ Instruction.Register2 ].AsInteger;
    
    // integer division can cause a HW error (division by zero)
    if( Divisor == 0 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::DivisionError );
        return;
    }
    
    // now perform division normally
    VirconWord* DividendRegister = &CPU.Registers[ Instruction.Register1 ];
    DividendRegister->AsInteger /= Divisor;
}

// -----------------------------------------------------------------------------

void ProcessIMOD( VirconCPU& CPU, CPUInstruction Instruction )
{
    // determine the operands
    VirconWord* DividendRegister = &CPU.Registers[ Instruction.Register1 ];
    int32_t Divisor = 1;
    
    if( Instruction.UsesImmediate )
      Divisor = CPU.ImmediateValue.AsInteger;
    else
      Divisor = CPU.Registers[ Instruction.Register2 ].AsInteger;
    
    // integer division can cause a HW error (division by zero)
    if( Divisor == 0 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::DivisionError );
        return;
    }
    
    // now perform modulo normally
    DividendRegister->AsInteger %= Divisor;
}

// -----------------------------------------------------------------------------

void ProcessISGN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsInteger = -Register1->AsInteger;
}

// -----------------------------------------------------------------------------

void ProcessIABS( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsInteger = abs( Register1->AsInteger );
}

// -----------------------------------------------------------------------------

void ProcessIMIN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsInteger = min( Register1->AsInteger, CPU.ImmediateValue.AsInteger );
    
    else
    {
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        Register1->AsInteger = min( Register1->AsInteger, Register2->AsInteger );
    }
}

// -----------------------------------------------------------------------------

void ProcessIMAX( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsInteger = max( Register1->AsInteger, CPU.ImmediateValue.AsInteger );
    
    else
    {
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        Register1->AsInteger = max( Register1->AsInteger, Register2->AsInteger );
    }
}

// -----------------------------------------------------------------------------

void ProcessFADD( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsFloat += CPU.ImmediateValue.AsFloat;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsFloat += SourceRegister->AsFloat;
    }
}

// -----------------------------------------------------------------------------

void ProcessFSUB( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsFloat -= CPU.ImmediateValue.AsFloat;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsFloat -= SourceRegister->AsFloat;
    }
}

// -----------------------------------------------------------------------------

void ProcessFMUL( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      DestinationRegister->AsFloat *= CPU.ImmediateValue.AsFloat;
    
    else
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        DestinationRegister->AsFloat *= SourceRegister->AsFloat;
    }
}

// -----------------------------------------------------------------------------

void ProcessFDIV( VirconCPU& CPU, CPUInstruction Instruction )
{
    // choose the requested divisor
    float Divisor = 1.0f;
    
    if( Instruction.UsesImmediate )
      Divisor = CPU.ImmediateValue.AsFloat;
    else
      Divisor = CPU.Registers[ Instruction.Register2 ].AsFloat;
    
    // float division can cause a HW exception (division by zero)
    if( Divisor == 0 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::DivisionError );
        return;
    }
    
    // now perform division normally
    VirconWord* DividendRegister = &CPU.Registers[ Instruction.Register1 ];
    DividendRegister->AsFloat /= Divisor;
}

// -----------------------------------------------------------------------------

void ProcessFMOD( VirconCPU& CPU, CPUInstruction Instruction )
{
    // choose the requested divisor
    float Divisor = 1.0f;
    
    if( Instruction.UsesImmediate )
      Divisor = CPU.ImmediateValue.AsFloat;
    else
      Divisor = CPU.Registers[ Instruction.Register2 ].AsFloat;
    
    // float modulus can cause a HW exception (division by zero)
    if( Divisor == 0 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::DivisionError );
        return;
    }
    
    // now perform modulus normally
    VirconWord* DividendRegister = &CPU.Registers[ Instruction.Register1 ];
    DividendRegister->AsFloat = fmod( DividendRegister->AsFloat, Divisor );
}

// -----------------------------------------------------------------------------

void ProcessFSGN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = -Register1->AsFloat;
}

// -----------------------------------------------------------------------------

void ProcessFABS( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = abs( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessFMIN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsFloat = min( Register1->AsFloat, CPU.ImmediateValue.AsFloat );
    
    else
    {
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        Register1->AsFloat = min( Register1->AsFloat, Register2->AsFloat );
    }
}

// -----------------------------------------------------------------------------

void ProcessFMAX( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    if( Instruction.UsesImmediate )
      Register1->AsFloat = max( Register1->AsFloat, CPU.ImmediateValue.AsFloat );
    
    else
    {
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        Register1->AsFloat = max( Register1->AsFloat, Register2->AsFloat );
    }
}

// -----------------------------------------------------------------------------

void ProcessFLR( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = floor( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessCEIL( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = ceil( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessROUND( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = round( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessSIN( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    Register1->AsFloat = sin( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessACOS( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    float Operand = Register1->AsFloat;
    
    // check that the operand is in range
    if( Operand < -1 || Operand > 1 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::ArcCosineError );
        return;
    }
    
    Register1->AsFloat = acos( Operand );
}

// -----------------------------------------------------------------------------

void ProcessATAN2( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    
    // check that the operands are not both zero
    if( !Register1->AsFloat && !Register2->AsFloat )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::ArcTangent2Error );
        return;
    }
    
    Register1->AsFloat = atan2( Register1->AsFloat, Register2->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessLOG( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    
    // check that the operand is positive
    if( Register1->AsFloat <= 0 )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::LogarithmError );
        return;
    }
    
    Register1->AsFloat = log( Register1->AsFloat );
}

// -----------------------------------------------------------------------------

void ProcessPOW( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    
    // check that there is a real solution
    if( Register1->AsFloat < 0 && trunc( Register2->AsFloat ) != Register2->AsFloat )
    {
        CPU.RaiseHardwareError( CPUErrorCodes::PowerError );
        return;
    }
    
    Register1->AsFloat = pow( Register1->AsFloat, Register2->AsFloat );
}


// =============================================================================
//      MOV VARIANTS PROCESSORS
// =============================================================================


void ProcessMOVRegFromImm( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    *Register1 = CPU.ImmediateValue;
}

// -----------------------------------------------------------------------------

void ProcessMOVRegFromReg( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    *Register1 = *Register2;
}

// -----------------------------------------------------------------------------

void ProcessMOVRegFromImmAdd( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    CPU.MemoryBus->ReadAddress( CPU.ImmediateValue.AsInteger, *Register1 );
}

// -----------------------------------------------------------------------------

void ProcessMOVRegFromRegAdd( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    CPU.MemoryBus->ReadAddress( Register2->AsInteger, *Register1 );
}

// -----------------------------------------------------------------------------

void ProcessMOVRegFromAddOff( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    CPU.MemoryBus->ReadAddress( Register2->AsInteger + CPU.ImmediateValue.AsInteger, *Register1 );
}

// -----------------------------------------------------------------------------

void ProcessMOVImmAddFromReg( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    CPU.MemoryBus->WriteAddress( CPU.ImmediateValue.AsInteger, *Register2 );
}

// -----------------------------------------------------------------------------

void ProcessMOVRegAddFromReg( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    CPU.MemoryBus->WriteAddress( Register1->AsInteger, *Register2 );
}

// -----------------------------------------------------------------------------

void ProcessMOVAddOffFromReg( VirconCPU& CPU, CPUInstruction Instruction )
{
    VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
    VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
    CPU.MemoryBus->WriteAddress( Register1->AsInteger + CPU.ImmediateValue.AsInteger, *Register2 );
}
