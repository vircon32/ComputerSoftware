// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "VirconASMLexer.hpp"
    #include "VirconASMPreprocessor.hpp"
    #include "VirconASMParser.hpp"
    #include "VirconASMEmitter.hpp"
    #include "Globals.hpp"
    #include "DebugInfo.hpp"
    
    // include C/C++ headers
    #include <fstream>      // [ C++ STL ] File streams
    #include <vector>       // [ C++ STL ] Vectors
    #include <cstring>      // [ ANSI C ] Strings
    
    // include SDL headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
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
    cout << "USAGE: assemble [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  --debugmode  Creates files with results of internal stages" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -b           Assembles the code as a BIOS" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
    cout << "  -g           Outputs an additional file with debug info" << endl;
    cout << "Also, the following options are accepted for compatibility" << endl;
    cout << "but have no effect: -s" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "assemble v25.1.4" << endl;
    cout << "Vircon32 assembler by Javier Carracedo" << endl;
}

// -----------------------------------------------------------------------------

// use this funcion to get the executable path
// in a portable way (can't be done without libraries)
string GetProgramFolder()
{
    if( SDL_Init( 0 ) )
      throw runtime_error( "cannot initialize SDL" );
    
    char* SDLString = SDL_GetBasePath();
    string Result = SDLString;
    
    SDL_free( SDLString );
    SDL_Quit();
    
    return Result;
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
        
        // capture assembler folder
        AssemblerFolder = GetProgramFolder();
        
        // if it is empty, we need at least a dot
        if( AssemblerFolder == "" || AssemblerFolder == string(1,PathSeparator) )
          AssemblerFolder = string(".") + PathSeparator;
        
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
            
            if( Arguments[i] == string("-g") )
            {
                CreateDebugVersion = true;
                continue;
            }
            
            if( Arguments[i] == string("--debugmode") )
            {
                DebugMode = true;
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
        
        // report when we are creating a debug binary
        if( VerboseMode && CreateDebugVersion )
          cout << "assembler will output debug information of the binary" << endl;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // simple compilation checks to ensure correct ROM creation
        if( sizeof( CPUInstruction ) != 4 )
          throw runtime_error( "ABI is incorrect: CPU instructions must be 4 bytes in size" );
          
        if( sizeof( float ) != 4 )
          throw runtime_error( "ABI is incorrect: floating point numbers must be 4 bytes in size" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 1: Run lexer
        // (Text --> List of tokens)
        if( VerboseMode )
          cout << "stage 1: running lexer" << endl;
        
        VirconASMLexer Lexer;
        Lexer.TokenizeFile( InputPath );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 2: Run preprocessor
        // (Token sequence --> Token sequence)
        if( VerboseMode )
          cout << "stage 2: running preprocessor" << endl;
          
        VirconASMPreprocessor Preprocessor;
        Preprocessor.Preprocess( Lexer );
        
        // when requested, log results of lexer + preprocessor stages
        if( DebugMode )
          SaveLexerLog( OutputPath + ".lexer.log", Preprocessor );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 3: Run parser
        // (Token sequence --> List of AST statement nodes)
        if( VerboseMode )
          cout << "stage 3: running parser" << endl;
          
        VirconASMParser Parser;
        Parser.ParseTopLevel( Preprocessor.ProcessedTokens );
        
        // when requested, log results of parser stage
        if( DebugMode )
          SaveParserLog( OutputPath + ".parser.log", Parser );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 4: Run emitter
        // (AST nodes --> binary ROM)
        if( VerboseMode )
          cout << "stage 4: running emitter" << endl;
        
        VirconASMEmitter Emitter;
        Emitter.Emit( Parser.ProgramAST );
        
        // when requested, log results of emitter stage
        if( DebugMode )
          SaveEmitterLog( OutputPath + ".emitter.log", Parser );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // open output file, in binary!
        // otherwise it replaces bytes '\n' with '\r\n', breaking the ROM
        if( VerboseMode )
          cout << "saving binary file" << endl;
        
        ofstream OutputFile;
        OpenOutputFile( OutputFile, OutputPath, ios_base::out | ios_base::binary );
        
        if( OutputFile.fail() )
          throw runtime_error( "cannot open output file \"" + OutputPath + "\"" );
        
        // determine program size
        uint32_t ROMSizeInWords = Emitter.ROM.size();
        uint32_t ROMSizeInBytes = ROMSizeInWords * 4;
        
        // create the VBIN file header
        BinaryFileFormat::Header VBINHeader;
        memcpy( VBINHeader.Signature, BinaryFileFormat::Signature, 8 );
        VBINHeader.NumberOfWords = ROMSizeInWords;
        
        // write the header in the file
        OutputFile.seekp( 0, ios_base::beg );
        OutputFile.write( (char*)(&VBINHeader), sizeof(BinaryFileFormat::Header) );
        
        // now add the whole ROM to the output
        OutputFile.write( (char*)(&Emitter.ROM[0]), ROMSizeInBytes );
        
        // close output
        OutputFile.close();
        
        // finally, report size of the produced ROM
        if( VerboseMode )
          cout << "output file created, size: " << ROMSizeInWords << " dwords = " << ROMSizeInBytes << " bytes" << endl;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // on debug assembly output an additional debug info file
        if( CreateDebugVersion )
          SaveDebugInfoFile( OutputPath + ".debug", Parser, Emitter );
    }
    
    catch( const exception& e )
    {
        cerr << "assemble: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "assembly successful" << endl;
    
    return 0;
}
