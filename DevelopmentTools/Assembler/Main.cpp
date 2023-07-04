// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "VirconASMLexer.hpp"
    #include "VirconASMParser.hpp"
    #include "VirconASMEmitter.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <fstream>      // [ C++ STL ] File streams
    #include <cstring>      // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: assemble [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays compiler version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -b           Assembles the code as a BIOS" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
    cout << "Also, the following options are accepted for compatibility" << endl;
    cout << "but have no effect: -s" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "assemble v23.1.16" << endl;
    cout << "Vircon32 assembler by Javier Carracedo" << endl;
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char* Arguments[] )
{
    try
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process command line arguments
        
        // variables to capture input parameters
        string InputPath, OutputPath;
        
        // process arguments
        for( int i = 1; i < NumberOfArguments; i++ )
        {
            if( Arguments[i] == string("--help") )
            {
                PrintUsage();
                return 0;
            }
            
            if( Arguments[i] == string("--version") )
            {
                PrintVersion();
                return 0;
            }
            
            if( Arguments[i] == string("-v") )
            {
                VerboseMode = true;
                continue;
            }
            
            if( Arguments[i] == string("-o") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing filename after '-o'" );
                
                // now we can safely read the input path
                OutputPath = Arguments[ i ];
                continue;
            }
            
            if( Arguments[i] == string("-b") )
            {
                InitialROMAddress = Constants::BiosProgramROMFirstAddress;
                continue;
            }
            
            // these options are accepted but have no effect
            if( Arguments[i] == string("-s")  )  continue;
            
            // discard any other parameters starting with '-'
            if( Arguments[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + Arguments[i] + "'" );
            
            // any non-option parameter is taken as the input file
            if( InputPath.empty() )
            {
                InputPath = Arguments[i];
                continue;
            }
            
            // only a single input file is supported!
            else
              throw runtime_error( "too many input files, only 1 is supported" );
        }
        
        // check if an input path was given
        if( InputPath.empty() )
          throw runtime_error( "no input file" );
        
        // if output path was not given, just
        // replace the extension in the input
        if( OutputPath.empty() )
        {
            OutputPath = ReplaceFileExtension( InputPath, "vbin" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // simple compilation checks to ensure correct ROM creation
        if( sizeof( CPUInstruction ) != 4 )
          throw runtime_error( "ABI is incorrect: CPU instructions must be 4 bytes in size" );
          
        if( sizeof( float ) != 4 )
          throw runtime_error( "ABI is incorrect: floating point numbers must be 4 bytes in size" );
        
        // open the file as text
        ifstream InputFile;
        InputFile.open( InputPath, ios_base::in );
        InputFile.seekg( 0, ios::beg );
        
        if( InputFile.fail() )
          throw runtime_error( "cannot open input file \"" + InputPath + "\"" );
          
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 1: Run lexer
        // (Text --> List of tokens)
        if( VerboseMode )
          cout << "stage 1: running lexer" << endl;
        
        VirconASMLexer Lexer;
        Lexer.ReadTokens( InputFile );
        
        // we are finished with the input file
        InputFile.close();
        
        // DEBUG: log all tokens, with their line numbers
        if( Debug )
          for( auto T : Lexer.Tokens )
          {
               cout << "[" + to_string( T->LineInSource ) + "] ";
               cout << T->ToString() << endl;
          }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 2: Run parser
        // (Token sequence --> List of AST statement nodes)
        if( VerboseMode )
          cout << "stage 2: running parser" << endl;
          
        VirconASMParser Parser;
        Parser.ParseTopLevel( Lexer.Tokens );
        
        // DEBUG: log full AST
        if( Debug )
          for( ASTNode* Node: Parser.ProgramAST )
            cout << Node->ToString() << endl;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 3: Run assembler
        // (AST nodes --> binary ROM)
        if( VerboseMode )
          cout << "stage 3: running emitter" << endl;
        
        VirconASMEmitter Emitter;
        Emitter.Emit( Parser.ProgramAST );
        
        // DEBUG: log all label addresses
        if( Debug )
          for( auto Pair : Emitter.LabelAddresses )
            cout << Pair.first << " -> " << Hex( Pair.second, 4 ) << endl;
        
        // DEBUG: log all node addresses
        if( Debug )
          for( ASTNode* Node: Parser.ProgramAST )
            cout << (Node->AddressInROM - InitialROMAddress) << ": " << Node->ToString() << endl;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // open output file, in binary!
        // otherwise it replaces bytes '\n' with '\r\n', breaking the ROM
        if( VerboseMode )
          cout << "saving binary file" << endl;
        
        ofstream OutputFile;
        OutputFile.open( OutputPath, ios_base::out | ios_base::binary );
        
        if( OutputFile.fail() )
          throw runtime_error( "cannot open output file \"" + OutputPath + "\"" );
        
        // determine program size
        uint32_t ROMSizeInWords = Emitter.ROM.size();
        uint32_t ROMSizeInBytes = ROMSizeInWords * 4;
        
        // create the VBIN file header
        BinaryFileHeader VBINHeader;
        memcpy( VBINHeader.Signature, Signatures::BinaryFile, 8 );
        VBINHeader.NumberOfWords = ROMSizeInWords;
        
        // write the header in the file
        OutputFile.seekp( 0, ios_base::beg );
        OutputFile.write( (char*)(&VBINHeader), sizeof(BinaryFileHeader) );
        
        // now add the whole ROM to the output
        OutputFile.write( (char*)(&Emitter.ROM[0]), ROMSizeInBytes );
        
        // close output
        OutputFile.close();
        
        // finally, check size of the produced ROM
        if( VerboseMode )
          cout << "output file created, size: " << ROMSizeInWords << " dwords = " << ROMSizeInBytes << " bytes" << endl;
    }
    
    catch( const exception& e )
    {
        cerr << "assemble: error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

