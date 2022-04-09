// *****************************************************************************
    // start include guard
    #ifndef ASMEMITFUNCTIONS_HPP
    #define ASMEMITFUNCTIONS_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // use forward declarations to avoid dependencies
    class VirconASMEmitter;
    class InstructionNode;
// *****************************************************************************


// we keep them outside the Assembler class to make
// it easier to use an [instruction -> emitter] map.
// However these functions do depend on Assembler
// since they use their internal methods  especially
// for the translation of labels to addresses.


// =============================================================================
//      EMIT FUNCTIONS FOR SPECIFIC NODES
// =============================================================================


void EmitHLT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitWAIT ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitJMP  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCALL ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitRET  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitJT   ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitJF   ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIEQ  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitINE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIGT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIGE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitILT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitILE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFEQ  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFNE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFGT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFGE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFLT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFLE  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitMOV  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitLEA  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitPUSH ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitPOP  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIN   ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitOUT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitMOVS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitSETS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCMPS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCIF  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCFI  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCIB  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCFB  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitNOT  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitAND  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitOR   ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitXOR  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitBNOT ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitSHL  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIADD ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitISUB ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIMUL ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIDIV ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIMOD ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitISGN ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIMIN ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIMAX ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitIABS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFADD ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFSUB ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFMUL ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFDIV ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFMOD ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFSGN ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFMIN ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFMAX ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFABS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitFLR  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitCEIL ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitROUND( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitSIN  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitACOS ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitATAN2( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitLOG  ( VirconASMEmitter &Emitter, InstructionNode& Node );
void EmitPOW  ( VirconASMEmitter &Emitter, InstructionNode& Node );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

