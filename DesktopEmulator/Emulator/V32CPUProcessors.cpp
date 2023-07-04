// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include project headers
    #include "V32CPU.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Mathematics
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      AUXILIARY FUNCTIONS
    // =============================================================================
    
    
    inline void Push( V32CPU& CPU, VirconWord Value )
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
    
    inline void Pop( V32CPU& CPU, VirconWord& Register )
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
    //      INSTRUCTION PROCESS FUNCTIONS FOR V32 CPU
    // =============================================================================
    
    
    void ProcessHLT( V32CPU& CPU, CPUInstruction Instruction )
    {
        CPU.Halted = true;
        cout << "CPU halted" << endl;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessWAIT( V32CPU& CPU, CPUInstruction Instruction )
    {
        CPU.Waiting = true;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessJMP( V32CPU& CPU, CPUInstruction Instruction )
    {
        if( Instruction.UsesImmediate )
          CPU.InstructionPointer = CPU.ImmediateValue;
        else
          CPU.InstructionPointer = CPU.Registers[ Instruction.Register1 ];
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessCALL( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessRET( V32CPU& CPU, CPUInstruction Instruction )
    {
        // pop the program counter
        Pop( CPU, CPU.InstructionPointer );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessJT( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessJF( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIEQ( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessINE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIGT( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIGE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessILT( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessILE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFEQ( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFNE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFGT( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFGE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFLT( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFLE( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessMOV( V32CPU& CPU, CPUInstruction Instruction )
    {
        // this is just a dummy function: MOV is not processed here;
        // however, removing it might hinder optimization for for the instruction switch
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessLEA( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        
        if( Instruction.UsesImmediate )
          Register1->AsInteger = Register2->AsInteger + CPU.ImmediateValue.AsInteger;
        
        else
          Register1->AsInteger = Register2->AsInteger;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessPUSH( V32CPU& CPU, CPUInstruction Instruction )
    {
        Push( CPU, CPU.Registers[ Instruction.Register1 ] );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessPOP( V32CPU& CPU, CPUInstruction Instruction )
    {
        Pop( CPU, CPU.Registers[ Instruction.Register1 ] );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessIN( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* DestinationRegister = &CPU.Registers[ Instruction.Register1 ];
        CPU.ControlBus->ReadPort( Instruction.PortNumber, *DestinationRegister );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessOUT( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* SourceRegister = &CPU.Registers[ Instruction.Register2 ];
        
        if( Instruction.UsesImmediate )
          CPU.ControlBus->WritePort( Instruction.PortNumber, CPU.ImmediateValue );
        else
          CPU.ControlBus->WritePort( Instruction.PortNumber, *SourceRegister );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVS( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessSETS( V32CPU& CPU, CPUInstruction Instruction )
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
    void ProcessCMPS( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessCIF( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsFloat = (float)Register->AsInteger;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessCFI( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsInteger = (int32_t)Register->AsFloat;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessCIB( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsInteger = (bool)Register->AsInteger;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessCFB( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsInteger = (bool)Register->AsFloat;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessNOT( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsBinary = ~Register->AsBinary;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessAND( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        
        if( Instruction.UsesImmediate )
          Register1->AsBinary &= CPU.ImmediateValue.AsBinary;
        else
          Register1->AsBinary &= CPU.Registers[ Instruction.Register2 ].AsBinary;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessOR( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        
        if( Instruction.UsesImmediate )
          Register1->AsBinary |= CPU.ImmediateValue.AsBinary;
        else
          Register1->AsBinary |= CPU.Registers[ Instruction.Register2 ].AsBinary;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessXOR( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        
        if( Instruction.UsesImmediate )
          Register1->AsBinary ^= CPU.ImmediateValue.AsBinary;
        else
          Register1->AsBinary ^= CPU.Registers[ Instruction.Register2 ].AsBinary;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessBNOT( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register = &CPU.Registers[ Instruction.Register1 ];
        Register->AsBinary = (Register->AsBinary? 0 : 1);
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessSHL( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIADD( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessISUB( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIMUL( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIDIV( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIMOD( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessISGN( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsInteger = -Register1->AsInteger;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessIABS( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsInteger = abs( Register1->AsInteger );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessIMIN( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessIMAX( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFADD( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFSUB( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFMUL( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFDIV( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFMOD( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFSGN( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = -Register1->AsFloat;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessFABS( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = abs( Register1->AsFloat );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessFMIN( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFMAX( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessFLR( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = floor( Register1->AsFloat );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessCEIL( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = ceil( Register1->AsFloat );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessROUND( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = round( Register1->AsFloat );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessSIN( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        Register1->AsFloat = sin( Register1->AsFloat );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessACOS( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessATAN2( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessLOG( V32CPU& CPU, CPUInstruction Instruction )
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
    
    void ProcessPOW( V32CPU& CPU, CPUInstruction Instruction )
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
    
    
    void ProcessMOVRegFromImm( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        *Register1 = CPU.ImmediateValue;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVRegFromReg( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        *Register1 = *Register2;
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVRegFromImmAdd( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        CPU.MemoryBus->ReadAddress( CPU.ImmediateValue.AsInteger, *Register1 );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVRegFromRegAdd( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        CPU.MemoryBus->ReadAddress( Register2->AsInteger, *Register1 );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVRegFromAddOff( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        CPU.MemoryBus->ReadAddress( Register2->AsInteger + CPU.ImmediateValue.AsInteger, *Register1 );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVImmAddFromReg( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        CPU.MemoryBus->WriteAddress( CPU.ImmediateValue.AsInteger, *Register2 );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVRegAddFromReg( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        CPU.MemoryBus->WriteAddress( Register1->AsInteger, *Register2 );
    }
    
    // -----------------------------------------------------------------------------
    
    void ProcessMOVAddOffFromReg( V32CPU& CPU, CPUInstruction Instruction )
    {
        VirconWord* Register1 = &CPU.Registers[ Instruction.Register1 ];
        VirconWord* Register2 = &CPU.Registers[ Instruction.Register2 ];
        CPU.MemoryBus->WriteAddress( Register1->AsInteger + CPU.ImmediateValue.AsInteger, *Register2 );
    }
}
