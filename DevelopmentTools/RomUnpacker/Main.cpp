// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/Definitions.hpp"
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "RomDefinition.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <fstream>      // [ C++ STL ] File streams
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <sys/stat.h>   // [ ANSI C ] File status
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


bool Debug = false;
bool VerboseMode = false;


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: unpackrom [options] inputfile outputfolder" << endl;
    cout << "InputFile: path to the Vircon32 rom file to unpack" << endl;
    cout << "OutputFolder: path to output folder to place rom definition and assets" << endl;
    cout << "(the output folder is created if it did not exist" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "unpackrom v24.8.4" << endl;
    cout << "Vircon32 ROM unpacker by Javier Carracedo" << endl;
}

// -----------------------------------------------------------------------------

void PerformABIAssertions()
{
    V32Word TestWord = {0};
    
    // determine the correct packing sizes
    if( sizeof(V32Word) != 4 )
      throw runtime_error( "ABI check failed: Vircon words are not 4 bytes in size" );
    
    // determine the correct bit endianness: instructions
    TestWord.AsInstruction.OpCode = 0x1;
    
    if( TestWord.AsBinary != 0x04000000 )
      throw runtime_error( "ABI check failed: Fields of CPU instructions are not correctly ordered" );
    
    // determine the correct byte endianness
    TestWord.AsColor.R = 0x11;
    TestWord.AsColor.G = 0x22;
    TestWord.AsColor.B = 0x33;
    TestWord.AsColor.A = 0x44;
    
    if( TestWord.AsBinary != 0x44332211 )
      throw runtime_error( "ABI check failed: Components GPU colors are not correctly ordered as RGBA" );
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
            
            // discard any other parameters starting with '-'
            if( Arguments[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + Arguments[i] + "'" );
            
            // the first non-option parameter is taken as the input file
            if( InputPath.empty() )
              InputPath = Arguments[i];
            
            // the second non-option parameter is taken as the output folder
            else if( OutputPath.empty() )
              OutputPath = Arguments[i];
            
            // other parameters are not supported
            else throw runtime_error( "too many non-option parameters" );
        }
        
        // check if an input path was given
        if( InputPath.empty() )
          throw runtime_error( "no input file" );
        
        // check if an output path was given
        if( OutputPath.empty() )
          throw runtime_error( "no output folder" );
        
        // check that output path is not a file
        if( FileExists( OutputPath ) )
          throw runtime_error( "output path is a file, but should be a folder" );
        
        // create output folder if it does not exist
        if( !DirectoryExists( OutputPath ) )
        {
            if( VerboseMode )
              cout << "creating output folder: \"" << OutputPath << "\"" << endl;
            
            int Status = mkdir( OutputPath.c_str() );
            
            if( Status < 0 )
              throw runtime_error( "Cannot create output folder" );
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Unpack the input ROM
        
        // do this test before anything else
        PerformABIAssertions();
        
        // use our rom definition class to unpack
        if( VerboseMode )
          cout << "unpacking ROM contents into output folder" << endl;
        
        RomDefinition Definition;
        Definition.UnpackROM( InputPath, OutputPath );
    }
    
    catch( const exception& e )
    {
        cerr << "unpackrom: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "unpacking successful" << endl;
    
    return 0;
}
