// *****************************************************************************
    // start include guard
    #ifndef V32CPU_HPP
    #define V32CPU_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include project headers
    #include "V32Buses.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      V32 CPU CLASS
    // =============================================================================
    
    
    class V32CPU
    {
        public:
            
            // general purpose registers
            VirconWord Registers[ 11 ];
            VirconWord CountRegister;       // alias for Registers[ 11 ]
            VirconWord SourceRegister;      // alias for Registers[ 12 ]
            VirconWord DestinationRegister; // alias for Registers[ 13 ]
            VirconWord BasePointer;         // alias for Registers[ 14 ]
            VirconWord StackPointer;        // alias for Registers[ 15 ]
            
            // not accessible registers
            VirconWord InstructionPointer;
            CPUInstruction Instruction;
            VirconWord ImmediateValue;
            
            // control flags
            bool Halted;
            bool Waiting;
            
        public:
            
            // connections with the host Vircon system
            V32MemoryBus* MemoryBus;
            V32ControlBus* ControlBus;
            
        public:
            
            // instance handling
            V32CPU();
            
            // general operation
            void Reset();
            void ChangeFrame();
            void RunNextCycle();
            
            // error handler
            void RaiseHardwareError( CPUErrorCodes Code );
    };
    
    
    // =============================================================================
    //      SPECIFIC INSTRUCTION PROCESSORS
    // =============================================================================
    
    
    void ProcessHLT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessWAIT ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessJMP  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCALL ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessRET  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessJT   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessJF   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIEQ  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessINE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIGT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIGE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessILT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessILE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFEQ  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFNE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFGT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFGE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFLT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFLE  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOV  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessLEA  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessPUSH ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessPOP  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIN   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessOUT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessSETS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCMPS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCIF  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCFI  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCIB  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCFB  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessNOT  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessAND  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessOR   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessXOR  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessBNOT ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessSHL  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIADD ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessISUB ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIMUL ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIDIV ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIMOD ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessISGN ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIMIN ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIMAX ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessIABS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFADD ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFSUB ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFMUL ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFDIV ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFMOD ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFSGN ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFMIN ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFMAX ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFABS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessFLR  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessCEIL ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessROUND( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessSIN  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessACOS ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessATAN2( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessLOG  ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessPOW  ( V32CPU& CPU, CPUInstruction Instruction );
    
    
    // =============================================================================
    //      MOV INSTRUCTION PROCESSORS
    // =============================================================================
    
    
    void ProcessMOVRegFromImm   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVRegFromReg   ( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVRegFromImmAdd( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVRegFromRegAdd( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVRegFromAddOff( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVImmAddFromReg( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVRegAddFromReg( V32CPU& CPU, CPUInstruction Instruction );
    void ProcessMOVAddOffFromReg( V32CPU& CPU, CPUInstruction Instruction );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
