// *****************************************************************************
    // include project headers
    #include "VirconASMEmitter.hpp"
    #include "ASMEmitFunctions.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>             // [ C++ STL ] I/O Streams
    #include <fstream>              // [ C++ STL ] File streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      MAPPING OF EACH OPCODE TO ITS SPECIFIC EMIT FUNCTION
// =============================================================================


typedef void (*EmitFunction)( VirconASMEmitter&, InstructionNode& );

// -----------------------------------------------------------------------------

EmitFunction InstructionEmitFunctions[] =
{
    EmitHLT,
    EmitWAIT,
    EmitJMP,
    EmitCALL,
    EmitRET,
    EmitJT,
    EmitJF,
    EmitIEQ,
    EmitINE,
    EmitIGT,
    EmitIGE,
    EmitILT,
    EmitILE,
    EmitFEQ,
    EmitFNE,
    EmitFGT,
    EmitFGE,
    EmitFLT,
    EmitFLE,
    EmitMOV,
    EmitLEA,
    EmitPUSH,
    EmitPOP,
    EmitIN,
    EmitOUT,
    EmitMOVS,
    EmitSETS,
    EmitCMPS,
    EmitCIF,
    EmitCFI,
    EmitCIB,
    EmitCFB,
    EmitNOT,
    EmitAND,
    EmitOR,
    EmitXOR,
    EmitBNOT,
    EmitSHL,
    EmitIADD,
    EmitISUB,
    EmitIMUL,
    EmitIDIV,
    EmitIMOD,
    EmitISGN,
    EmitIMIN,
    EmitIMAX,
    EmitIABS,
    EmitFADD,
    EmitFSUB,
    EmitFMUL,
    EmitFDIV,
    EmitFMOD,
    EmitFSGN,
    EmitFMIN,
    EmitFMAX,
    EmitFABS,
    EmitFLR,
    EmitCEIL,
    EmitROUND,
    EmitSIN,
    EmitACOS,
    EmitATAN2,
    EmitLOG,
    EmitPOW
};


// =============================================================================
//      VIRCON ASM EMITTER: INSTANCE HANDLING
// =============================================================================


VirconASMEmitter::VirconASMEmitter()
{
    ProgramAST = nullptr;
}


// =============================================================================
//      VIRCON ASM EMITTER: ERROR HANDLING
// =============================================================================


void VirconASMEmitter::EmitError( int LineInSource, const string& Description, bool Abort )
{
    cerr << "line " << LineInSource << ": ";
    cerr << "emitter error: " << Description << endl;
    
    if( Abort )
      throw runtime_error( "assembly terminated" );
}

// -----------------------------------------------------------------------------

void VirconASMEmitter::EmitWarning( int LineInSource, const string& Description )
{
    cerr << "line " << LineInSource << ": " << endl;
    cerr << "emitter warning: " << Description << endl;
}


// =============================================================================
//      VIRCON ASM EMITTER: HELPERS FOR EMIT FUNCTIONS
// =============================================================================


void VirconASMEmitter::CheckOperands( InstructionNode& Node, int NumberOfOperands )
{
    int ExistingOperands = Node.Operands.size();
    string OpCode = OpCodeToString( Node.OpCode );
    
    if( ExistingOperands < NumberOfOperands )
    {
        EmitError( Node.LineInSource, string("too few operands for instruction ")+ OpCode );
        throw runtime_error( "Aborted" );
    }
    
    else if( ExistingOperands > NumberOfOperands )
    {
        EmitError( Node.LineInSource, string("too many operands for instruction ")+ OpCode );
        throw runtime_error( "Aborted" );
    }
}

// -----------------------------------------------------------------------------

// first checks that the label exists!!
int32_t VirconASMEmitter::GetLabelAddress( ASTNode& ReferringNode, string LabelName )
{
    auto AddressPair = LabelAddresses.find( LabelName );
    
    if( AddressPair == LabelAddresses.end() )
    {
        EmitError( ReferringNode.LineInSource, string("label \"") + LabelName + "\" was not declared" );
        throw runtime_error( "Aborted" );
    }
    
    return AddressPair->second;
}

// -----------------------------------------------------------------------------

void VirconASMEmitter::ReadDataFile( DataFileNode& Node )
{
    // open the file
    ifstream InputFile;
    InputFile.open( Node.FilePath, ios_base::binary | ios_base::ate );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are probably wrong)
    unsigned FileSize = InputFile.tellg();
    
    if( (FileSize % 4) != 0 )
      EmitError( Node.LineInSource, "data file size must be a multiple of 4 to be inserted in a rom" );
    
    // read it into the vector
    Node.FileContents.resize( FileSize / 4 );
    InputFile.seekg( 0, ios_base::beg );
    InputFile.read( (char*)(&Node.FileContents[0]), FileSize );
    
    // close the file
    InputFile.close();
}

// -----------------------------------------------------------------------------

int32_t VirconASMEmitter::GetValueAsAddress( InstructionNode& Node, BasicValue& Value )
{
    string OpCodeName = OpCodeToString( Node.OpCode );
    
    // case 1: address as an integer
    if( Value.Type == BasicValueTypes::LiteralInteger )
      return Value.IntegerField;
    
    // case 2: address as a label
    if( Value.Type == BasicValueTypes::Label )
    {
        string LabelName = Value.LabelField;
        return GetLabelAddress( Node, LabelName );
    }
    
    EmitError( Node.LineInSource, OpCodeName + " expected a memory address (integer or label)" );
    
    // stop the warning
    return 0;
}

// -----------------------------------------------------------------------------

VirconWord VirconASMEmitter::GetValueAsImmediate( InstructionNode& Node, BasicValue& Value )
{
    string OpCodeName = OpCodeToString( Node.OpCode );
    VirconWord Result;
    
    // case 1: literal integer
    if( Value.Type == BasicValueTypes::LiteralInteger )
      Result.AsInteger = Value.IntegerField;
    
    // case 2: literal float
    else if( Value.Type == BasicValueTypes::LiteralFloat )
      Result.AsFloat = Value.FloatField;
    
    // case 3: label taken as integer address
    else if( Value.Type == BasicValueTypes::Label )
    {
        string LabelName = Value.LabelField;
        Result.AsInteger = GetLabelAddress( Node, LabelName );
    }
    
    // other cases
    else
      EmitError( Node.LineInSource, OpCodeName + " expected a numeric value (integer, float or label)" );

    return Result;
}


// =============================================================================
//      VIRCON ASM EMITTER: EMIT FROM GENERIC INSTRUCTION NODE
// =============================================================================


void VirconASMEmitter::EmitInstructionFromNode( InstructionNode& Node )
{
    InstructionEmitFunctions[ (int)Node.OpCode ]( *this, Node );
}


// =============================================================================
//      MAIN ASSEMBLY FUNCTION
// =============================================================================


void VirconASMEmitter::Emit( NodeList& ProgramAST_ )
{
    // restart assembly
    ProgramAST = &ProgramAST_;
    
    // delete any previous results
    ROM.clear();
    LabelAddresses.clear();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PASS 1: Allocate ROM addresses
    // (in the same pass, we will also locate all labels)
    uint32_t ROMAddress = InitialROMAddress;
    
    for( ASTNode* Node: *ProgramAST )
    {
        Node->AddressInROM = ROMAddress;
        
        if( Node->Type() == ASTNodeTypes::Instruction )
          ROMAddress += ((InstructionNode*)Node)->SizeInWords();
        
        else if( Node->Type() == ASTNodeTypes::IntegerData )
          ROMAddress += ((IntegerDataNode*)Node)->Values.size();
        
        else if( Node->Type() == ASTNodeTypes::FloatData )
          ROMAddress += ((FloatDataNode*)Node)->Values.size();
        
        else if( Node->Type() == ASTNodeTypes::StringData )
          ROMAddress += ((StringDataNode*)Node)->Value.size() + 1;  // strings are zero-terminated!
        
        else if( Node->Type() == ASTNodeTypes::Label )
        {
            string LabelName = ((LabelNode*)Node)->Name;
            
            // check for double declaration!
            if( LabelAddresses.find( LabelName ) !=  LabelAddresses.end() )
              EmitError( Node->LineInSource, "label \"" + LabelName + "\" has already been declared" );
            
            LabelAddresses[ LabelName ] = ROMAddress;
        }
        
        else if( Node->Type() == ASTNodeTypes::DataFile )
        {
            DataFileNode* DFN = (DataFileNode*)Node;
            ReadDataFile( *DFN );
            ROMAddress += DFN->FileContents.size();
        }
        
        // (do nothing with definition nodes:
        // (they neither occupy nor reference addresses)
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PASS 2: Emit binary ROM
    for( ASTNode* Node: *ProgramAST )
    {
        // CASE 1: Instructions -> Call its specialized function
        if( Node->Type() == ASTNodeTypes::Instruction )
        {
            InstructionNode* IN = (InstructionNode*)Node;
            EmitInstructionFromNode( *IN );
        }
        
        // CASE 2: Integers / Floats -> Add each value to the ROM
        else if( Node->Type() == ASTNodeTypes::IntegerData )
        {
            IntegerDataNode* IDN = (IntegerDataNode*)Node;
            
            for( int32_t Value: IDN->Values )
            {
                ROM.emplace_back();
                ROM.back().AsInteger = Value;
            }
        }
        
        else if( Node->Type() == ASTNodeTypes::FloatData )
        {
            FloatDataNode* FDN = (FloatDataNode*)Node;
            
            for( float Value: FDN->Values )
            {
                ROM.emplace_back();
                ROM.back().AsFloat = Value;
            }
        }
        
        // CASE 3: Strings -> Add each character to the ROM
        // converted to int32, and terminate string with a zero
        else if( Node->Type() == ASTNodeTypes::StringData )
        {
            StringDataNode* SDN = (StringDataNode*)Node;
            
            for( char c: SDN->Value )
            {
                union
                {
                    char AsNumber;
                    unsigned char AsBinary;
                }
                Character;
                
                ROM.emplace_back();
                Character.AsNumber = c;
                ROM.back().AsBinary = Character.AsBinary;
            }
            
            // add null termination
            ROM.emplace_back();
            ROM.back().AsBinary = 0;
        }
        
        // CASE 4: Data Files -> Add all its contents to ROM
        else if( Node->Type() == ASTNodeTypes::DataFile )
        {
            DataFileNode* DFN = (DataFileNode*)Node;
            
            for( VirconWord Word: DFN->FileContents )
              ROM.push_back( Word );
        }
        
        // (define nodes are ignored)
    }
}

