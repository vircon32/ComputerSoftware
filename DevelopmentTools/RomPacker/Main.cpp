// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconDataStructures.hpp"
    #include "../../VirconDefinitions/VirconROMFormat.hpp"
    
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
    
    // declare used namespaces
    using namespace std;
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
    cout << "USAGE: packrom [options] file" << endl;
    cout << "File: a rom definition in XML format" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "packrom v2022.04.09" << endl;
    cout << "Vircon32 ROM packer by Javier Carracedo" << endl;
}

// -----------------------------------------------------------------------------

void PerformABIAssertions()
{
    VirconWord TestWord = {0};
    
    // determine the correct packing sizes
    if( sizeof(VirconWord) != 4 )
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
            
            // any non-option parameter is taken as the input file
            if( InputPath.empty() )
            {
                InputPath = Arguments[i];
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
            OutputPath = ReplaceFileExtension( InputPath, "v32" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load input ROM definition as XML
        
        // do this test before anything else
        PerformABIAssertions();
        
        // determine folder of the input definition file
        // (since all files will be relative to it)
        RomDefinition Definition;
        Definition.BaseFolder = GetPathDirectory( InputPath );
        
        // load the XML file into our rom definition class
        if( VerboseMode )
          cout << "loading ROM definition from input file" << endl;
        
        Definition.LoadXML( InputPath );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Pack the output ROM
        
        if( VerboseMode )
          cout << "packing ROM contents into output file" << endl;
        
        Definition.PackROM( OutputPath );
    }
    
    catch( const exception& e )
    {
        cerr << "packrom: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "packing successful" << endl;
    
    return 0;
}
