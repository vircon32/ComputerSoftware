// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "VirconDisassembler.hpp"
    
    // include external headers
    #include <string>       // [ C++ STL ] Strings
    #include <fstream>      // [ C++ STL ] File streams
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    #include <vector>       // [ C++ STL ] Vectors
    
    // detection of Windows
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
    #endif
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: disassemble [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -b           Disassembles the code as a BIOS" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "disassemble v25.1.4" << endl;
    cout << "Vircon32 disassembler by Javier Carracedo" << endl;
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


// on Windows we need to use wmain to be able to receive
// unicode text from the console as input arguments; if
// we use regular main we can only process ASCII paths
#if defined(WINDOWS_OS)
  int wmain( int NumberOfArguments, wchar_t* ArgumentsUTF16[] )
#else
  int main( int NumberOfArguments, char* Arguments[] )
#endif
{
    try
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process command line arguments
        
        // variables to capture input parameters
        string InputPath, OutputPath;
        
        // on Windows convert all arguments to UTF-8 beforehand
        // (that way we can treat them the same as in other OSs)
        #if defined(WINDOWS_OS)
          vector< string > Arguments;
          
          for( int i = 1; i < NumberOfArguments; i++ )
            Arguments.push_back( ToUTF8( ArgumentsUTF16[i] ) );
        #endif
        
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
            OutputPath = ReplaceFileExtension( InputPath, "asm" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // simple compilation checks to ensure correct ROM reading
        if( sizeof( CPUInstruction ) != 4 )
          throw runtime_error( "ABI is incorrect: CPU instructions must be 4 bytes in size" );
          
        if( sizeof( float ) != 4 )
          throw runtime_error( "ABI is incorrect: floating point numbers must be 4 bytes in size" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // load the ROM from the input path
        VirconDisassembler Disassembler;
        Disassembler.LoadROM( InputPath );
        
        // now open output file
        ofstream OutputFile;
        OpenOutputFile( OutputFile, OutputPath, ios_base::out );
        
        if( OutputFile.fail() )
          throw runtime_error( "cannot open output file \"" + OutputPath + "\"" );
        
        // perform disassembly
        Disassembler.Disassemble( OutputFile, true );
        
        // close output
        OutputFile.close();
    }
    
    catch( const exception& e )
    {
        cerr << "disassemble: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "assembly successful" << endl;
    
    return 0;
}
