// *****************************************************************************
    // start include guard
    #ifndef OPERANDWRITERS_HPP
    #define OPERANDWRITERS_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include project headers
    #include "VirconDisassembler.hpp"
// *****************************************************************************


// =============================================================================
//      SPECIALIZED OPERAND WRITERS
// =============================================================================


std::string WriteHLT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteWAIT ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteJMP  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCALL ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteRET  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteJT   ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteJF   ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIEQ  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteINE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIGT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIGE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteILT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteILE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFEQ  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFNE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFGT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFGE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFLT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFLE  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteMOV  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteLEA  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WritePUSH ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WritePOP  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIN   ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteOUT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteMOVS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteSETS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCMPS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCIF  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCFI  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCIB  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCFB  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteNOT  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteAND  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteOR   ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteXOR  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteBNOT ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteSHL  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIADD ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteISUB ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIMUL ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIDIV ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIMOD ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteISGN ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIMIN ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIMAX ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteIABS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFADD ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFSUB ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFMUL ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFDIV ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFMOD ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFSGN ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFMIN ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFMAX ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFABS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteFLR  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteCEIL ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteROUND( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteSIN  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteACOS ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteATAN2( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WriteLOG  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );
std::string WritePOW  ( VirconDisassembler& Disassembler, V32::CPUInstruction Instruction, V32::V32Word ImmediateValue );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
