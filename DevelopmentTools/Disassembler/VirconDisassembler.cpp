// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    #include "../DevToolsInfrastructure/FileSignatures.hpp"
    
    // include project headers
    #include "VirconDisassembler.hpp"
    #include "OperandWriters.hpp"
    
    // include external headers
    #include <fstream>      // [ C++ STL ] File streams
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <sstream>      // [ C++ STL ] String Streams
    #include <iomanip>      // [ C++ STL ] I/O Manipulation
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      MAPPING OF EACH OPCODE TO ITS SPECIFIC WRITER FUNCTION
// =============================================================================


typedef std::string (*WriterFunction)
(
    VirconDisassembler& Disassembler,
    CPUInstruction Instruction,
    V32Word ImmediateValue
);

// -----------------------------------------------------------------------------

WriterFunction OperandWriteFunctions[] =
{
    WriteHLT,
    WriteWAIT,
    WriteJMP,
    WriteCALL,
    WriteRET,
    WriteJT,
    WriteJF,
    WriteIEQ,
    WriteINE,
    WriteIGT,
    WriteIGE,
    WriteILT,
    WriteILE,
    WriteFEQ,
    WriteFNE,
    WriteFGT,
    WriteFGE,
    WriteFLT,
    WriteFLE,
    WriteMOV,
    WriteLEA,
    WritePUSH,
    WritePOP,
    WriteIN,
    WriteOUT,
    WriteMOVS,
    WriteSETS,
    WriteCMPS,
    WriteCIF,
    WriteCFI,
    WriteCIB,
    WriteCFB,
    WriteNOT,
    WriteAND,
    WriteOR,
    WriteXOR,
    WriteBNOT,
    WriteSHL,
    WriteIADD,
    WriteISUB,
    WriteIMUL,
    WriteIDIV,
    WriteIMOD,
    WriteISGN,
    WriteIMIN,
    WriteIMAX,
    WriteIABS,
    WriteFADD,
    WriteFSUB,
    WriteFMUL,
    WriteFDIV,
    WriteFMOD,
    WriteFSGN,
    WriteFMIN,
    WriteFMAX,
    WriteFABS,
    WriteFLR,
    WriteCEIL,
    WriteROUND,
    WriteSIN,
    WriteACOS,
    WriteATAN2,
    WriteLOG,
    WritePOW
};


// =============================================================================
//      INSTRUCTION CLASSIFICATION FUNCTIONS
// =============================================================================


bool IsInconditionalJump( CPUInstruction& Instruction )
{
    if( Instruction.OpCode == (int)InstructionOpCodes::JMP ) return true;
    return false;
}

// -----------------------------------------------------------------------------

bool IsConditionalJump( CPUInstruction& Instruction )
{
    if( Instruction.OpCode == (int)InstructionOpCodes::JT ) return true;
    if( Instruction.OpCode == (int)InstructionOpCodes::JF ) return true;
    return false;
}

// -----------------------------------------------------------------------------

bool IsSubroutineCall( CPUInstruction& Instruction )
{
    if( Instruction.OpCode == (int)InstructionOpCodes::CALL ) return true;
    return false;
}

// -----------------------------------------------------------------------------

bool IsEndOfBranch( CPUInstruction& Instruction )
{
    if( Instruction.OpCode == (int)InstructionOpCodes::RET ) return true;
    if( Instruction.OpCode == (int)InstructionOpCodes::HLT ) return true;
    return false;
}


// =============================================================================
//      VIRCON DISASSEMBLER: DISASSEMBLY FUNCTIONS
// =============================================================================


void VirconDisassembler::LoadROM( const string& InputPath )
{
    // first, release any previous ROM
    ROM.clear();
    
    // open input file
    ifstream InputFile;
    InputFile.open( InputPath, ios_base::binary | ios_base::ate );
    
    if( InputFile.fail() )
      throw runtime_error( "cannot open input file \"" + InputPath + "\"" );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = InputFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "incorrect VBIN file format (file size must be a multiple of 4)" );
    
    // ensure that we can at least load the file header
    if( FileBytes < sizeof(BinaryFileFormat::Header) )
      throw runtime_error( "incorrect VBIN file format (file is too small)" );
    
    // now we can safely read the file header
    InputFile.seekg( 0, ios_base::beg );
    BinaryFileFormat::Header BinaryHeader;
    InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
    
    // check signature for binary file
    if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
      throw runtime_error( "incorrect VBIN file format (file does not have a valid signature)" );
    
    // check for the correct file size
    uint32_t ExpectedFileSize = sizeof(BinaryFileFormat::Header) + 4*BinaryHeader.NumberOfWords;
    
    if( FileBytes != ExpectedFileSize )
      throw runtime_error( "incorrect VBIN file format (file size does not match indicated binary size)" );
    
    // load the whole binary content as bytes
    ROM.resize( BinaryHeader.NumberOfWords );
    InputFile.read( (char*)(&ROM[0]), 4*BinaryHeader.NumberOfWords );
    
    // finally, close the file
    InputFile.close();
}

// -----------------------------------------------------------------------------

void VirconDisassembler::DisassembleBranch( uint32_t ROMIndex )
{
    while( ROMIndex < ROM.size() )
    {
        // end branch as soon as some instruction was already visited
        if( VisitedInstructions.find( ROMIndex ) != VisitedInstructions.end() )
          return;
        
        // fetch the instruction
        CPUInstruction Instruction = ROM[ ROMIndex ].AsInstruction;
        V32Word ImmediateWord = {0};
        
        // add this location to the visited instructions
        VisitedInstructions[ ROMIndex ] = Instruction;
        ROMIndex++;
        
        // obtain immediate value, if it is used
        if( Instruction.UsesImmediate )
        {
            ImmediateWord = ROM[ ROMIndex ];
            ROMIndex++;
        }
        
        // CASE 1: this branch has ended
        if( IsEndOfBranch( Instruction ) )
          return;
        
        // CASE 2: branching path for subroutine
        if( IsSubroutineCall( Instruction ) && Instruction.UsesImmediate )
        {
            uint32_t DestinationROMIndex = ImmediateWord.AsInteger - InitialROMAddress;
            JumpDestinationNames[ DestinationROMIndex ] = "";
            
            // recurse into the a branch for the sub
            DisassembleBranch( DestinationROMIndex );
        }
        
        // CASE 3: direct jump (same branch, but continued elsewhere)
        else if( IsInconditionalJump( Instruction ) && Instruction.UsesImmediate )
        {
            ROMIndex = ImmediateWord.AsInteger - InitialROMAddress;
            JumpDestinationNames[ ROMIndex ] = "";
        }
        
        // CASE 4: branching path for conditional jumps
        else if( IsConditionalJump( Instruction ) && Instruction.UsesImmediate )
        {
            uint32_t DestinationROMIndex = ImmediateWord.AsInteger - InitialROMAddress;
            JumpDestinationNames[ DestinationROMIndex ] = "";
            
            // recurse into the a branch for the sub
            DisassembleBranch( DestinationROMIndex );
        }
        
        // for other instructions, just continue
    }
}

// -----------------------------------------------------------------------------

void VirconDisassembler::Disassemble( ostream& Output, bool IncludeDescriptions )
{
    // find all accessible branches from ROM start
    // (i.e. cartridge ROM index 0, that corresponds to address 0x20000000 at runtime)
    DisassembleBranch( 0 );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now do a first pass to name all labels in order
    uint32_t ROMIndex = 0;
    uint32_t ROMSize = ROM.size();
    int LabelNumber = 1;

    while( ROMIndex < ROMSize )
    {
        auto VJD = JumpDestinationNames.find( ROMIndex );
        bool HasLabel = (VJD != JumpDestinationNames.end());
        
        if( HasLabel )
        {
            VJD->second = string("_label") + to_string( LabelNumber );
            LabelNumber++;
        }
        
        ROMIndex++;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // on second pass, actually output the results byte per byte
    
    ROMIndex = 0;
    bool PreviousIndexWasCode = true;
    
    while( ROMIndex < ROMSize )
    {
        // add an initial label if it is a jump destination
        auto VJD = JumpDestinationNames.find( ROMIndex );
        bool HasLabel = (VJD != JumpDestinationNames.end());
        
        if( HasLabel )
        {
            // add the ROM position as a comment
            Output << endl << "; ROM address " << Hex(InitialROMAddress + VJD->first, 8) << endl;
            
            // write the label
            Output << VJD->second << ":" << endl;
            
            LabelNumber++;
        }
        
        // check if there is an instruction here
        auto VIns = VisitedInstructions.find( ROMIndex );
        bool CurrentIndexIsCode = (VIns != VisitedInstructions.end());
        
        // separate code sections from data sections
        if( !HasLabel )
          if( PreviousIndexWasCode != CurrentIndexIsCode )
            Output << endl;
        
        // for instructions, write their disassembly
        // and optionally, a description of what they do
        if( CurrentIndexIsCode )
        {
            CPUInstruction Instruction = VIns->second;
            V32Word ImmediateValue = {0};
            ROMIndex++;
            
            if( Instruction.UsesImmediate )
            {
                ImmediateValue = ROM[ ROMIndex ];
                ROMIndex++;
            }
            
            Output << "  " << OpCodeToString( (InstructionOpCodes)Instruction.OpCode );
            Output << OperandWriteFunctions[ Instruction.OpCode ]( *this, Instruction, ImmediateValue );
            Output << endl;
        }
        
        // otherwise keep writing integers as data
        else
        {
            Output << "  integer ";
            int IntegersWritten = 0;
            
            while( ROMIndex < ROMSize )
            {
                if( IntegersWritten > 0 )
                  Output << ", ";
                
                int32_t Value = ROM[ ROMIndex ].AsInteger;
                Output << Hex( Value, 8 );
                IntegersWritten++;
                ROMIndex++;
                
                // separate groups of more than 10 integers
                if( IntegersWritten >= 10 )
                  break;
                
                // continue if next value is also data
                VIns = VisitedInstructions.find( ROMIndex );
                CurrentIndexIsCode = (VIns != VisitedInstructions.end());
                
                if( CurrentIndexIsCode )
                  break;
            }
            
            Output << endl;
        }
        
        PreviousIndexWasCode = CurrentIndexIsCode;
    }
}
