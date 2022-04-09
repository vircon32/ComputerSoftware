// *****************************************************************************
    // start include guard
    #ifndef VIRCONCPU_HPP
    #define VIRCONCPU_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include project headers
    #include "VirconBuses.hpp"
// *****************************************************************************


// =============================================================================
//      VIRCON CPU CLASS
// =============================================================================


class VirconCPU
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
        VirconMemoryBus* MemoryBus;
        VirconControlBus* ControlBus;
        
    public:
        
        // instance handling
        VirconCPU();
        
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


void ProcessHLT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessWAIT ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessJMP  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCALL ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessRET  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessJT   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessJF   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIEQ  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessINE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIGT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIGE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessILT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessILE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFEQ  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFNE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFGT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFGE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFLT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFLE  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOV  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessLEA  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessPUSH ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessPOP  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIN   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessOUT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessSETS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCMPS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCIF  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCFI  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCIB  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCFB  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessNOT  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessAND  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessOR   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessXOR  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessBNOT ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessSHL  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIADD ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessISUB ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIMUL ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIDIV ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIMOD ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessISGN ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIMIN ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIMAX ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessIABS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFADD ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFSUB ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFMUL ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFDIV ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFMOD ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFSGN ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFMIN ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFMAX ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFABS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessFLR  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessCEIL ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessROUND( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessSIN  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessACOS ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessATAN2( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessLOG  ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessPOW  ( VirconCPU& CPU, CPUInstruction Instruction );


// =============================================================================
//      MOV INSTRUCTION PROCESSORS
// =============================================================================


void ProcessMOVRegFromImm   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVRegFromReg   ( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVRegFromImmAdd( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVRegFromRegAdd( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVRegFromAddOff( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVImmAddFromReg( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVRegAddFromReg( VirconCPU& CPU, CPUInstruction Instruction );
void ProcessMOVAddOffFromReg( VirconCPU& CPU, CPUInstruction Instruction );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

