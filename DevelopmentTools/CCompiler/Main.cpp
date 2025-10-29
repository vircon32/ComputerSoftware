// *****************************************************************************
    // include vircon common headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "VirconCLexer.hpp"
    #include "VirconCPreprocessor.hpp"
    #include "VirconCParser.hpp"
    #include "VirconCAnalyzer.hpp"
    #include "VirconCEmitter.hpp"
    #include "CompilerInfrastructure.hpp"
    #include "Globals.hpp"
    #include "DebugInfo.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <fstream>          // [ C++ STL ] File streams
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <vector>           // [ C++ STL ] Vectors
    
    // include SDL headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
    // on Windows include headers for unicode conversion
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
      #include <windows.h>      // [ WINDOWS ] Main header
      #include <shellapi.h>     // [ WINDOWS ] Shell API
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: compile [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays compiler version" << endl;
    cout << "  --debugmode  Creates files with results of internal stages" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -b           Compiles the program as a BIOS" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
    cout << "  -g           Outputs an additional file with debug info" << endl;
    cout << "  -w           Inhibit all warnings" << endl;
    cout << "  -Wall        Enable all warnings" << endl;
    cout << "Also, the following options are accepted for compatibility" << endl;
    cout << "but have no effect: -c,-s,-O1,-O2,-O3" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "compile v25.10.29" << endl;
    cout << "Vircon32 C compiler by Javier Carracedo" << endl;
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


int main( int NumberOfArguments, char* Arguments[] )
{
    try
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process command line arguments
        
        // capture compiler folder
        CompilerFolder = GetProgramFolder();
        
        // if it is empty, we need at least a dot
        if( CompilerFolder == "" || CompilerFolder == string(1,PathSeparator) )
          CompilerFolder = string(".") + PathSeparator;
        
        // variables to capture input parameters
        string InputPath, OutputPath;
        bool ProgramIsBios = false;
        
        // to treat arguments the same in any OS we
        // will convert them to UTF-8 in all cases
        vector< string > ArgumentsUTF8;
        
        #if defined(WINDOWS_OS)
        
          // on Windows we can't rely on the arguments received
          // in main: ask Windows for the UTF-16 command line
          wchar_t* CommandLineUTF16 = GetCommandLineW();
          wchar_t** ArgumentsUTF16 = CommandLineToArgvW( CommandLineUTF16, &NumberOfArguments );
          
          // now convert every program argument to UTF-8
          for( int i = 0; i < NumberOfArguments; i++ )
            ArgumentsUTF8.push_back( ToUTF8( ArgumentsUTF16[i] ) );
          
          LocalFree( ArgumentsUTF16 );
          
        #else
            
          // on Linux/Mac arguments in main are already UTF-8
          for( int i = 0; i < NumberOfArguments; i++ )
            ArgumentsUTF8.push_back( Arguments[i] );
        
        #endif
        
        // process arguments
        for( int i = 1; i < NumberOfArguments; i++ )
        {
            if( ArgumentsUTF8[i] == string("--help") )
            {
                PrintUsage();
                return 0;
            }
            
            if( ArgumentsUTF8[i] == string("--version") )
            {
                PrintVersion();
                return 0;
            }
            
            if( ArgumentsUTF8[i] == string("-v") )
            {
                VerboseMode = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-c") )
            {
                CompileOnly = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-w") )
            {
                DisableWarnings = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-b") )
            {
                ProgramIsBios = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-Wall") )
            {
                EnableAllWarnings = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-g") )
            {
                CreateDebugVersion = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("--debugmode") )
            {
                DebugMode = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-o") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing filename after '-o'" );
                
                // now we can safely read the input path
                OutputPath = ArgumentsUTF8[ i ];
                continue;
            }
            
            // these options are accepted but have no effect
            if( ArgumentsUTF8[i] == string("-s")  )  continue;
            if( ArgumentsUTF8[i] == string("-O1") )  continue;
            if( ArgumentsUTF8[i] == string("-O2") )  continue;
            if( ArgumentsUTF8[i] == string("-O3") )  continue;
            
            // discard any other parameters starting with '-'
            if( ArgumentsUTF8[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + ArgumentsUTF8[i] + "'" );
            
            // any non-option parameter is taken as the input file
            if( InputPath.empty() )
            {
                InputPath = ArgumentsUTF8[i];
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
        // Begin a new compilation
        CompilationErrors = 0;
        CompilationWarnings = 0;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 1: Run lexer
        // (Text --> List of tokens)
        if( VerboseMode )
          cout << "stage 1: running lexer" << endl;
        
        VirconCLexer Lexer;
        Lexer.TokenizeFile( InputPath );
        
        // avoid later stages if any error was found
        if( CompilationErrors != 0 )
          throw runtime_error( "lexer finished with errors" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 2: Run preprocessor
        // (Token sequence --> Token sequence)
        if( VerboseMode )
          cout << "stage 2: running preprocessor" << endl;
          
        VirconCPreprocessor Preprocessor;
        Preprocessor.Preprocess( Lexer );
        
        // when requested, log results of lexer + preprocessor stages
        if( DebugMode )
          SaveLexerLog( OutputPath + ".lexer.log", Preprocessor );
        
        // avoid later stages if any error was found
        if( CompilationErrors != 0 )
          throw runtime_error( "preprocessor finished with errors" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 3: Run parser
        // (Token sequence --> List of AST statement nodes)
        if( VerboseMode )
          cout << "stage 3: running parser" << endl;
        
        VirconCParser Parser;
        Parser.ParseTopLevel( Preprocessor.ProcessedTokens );
        
        // when requested, log results of parser stage
        if( DebugMode )
          SaveParserLog( OutputPath + ".parser.log", Parser );
        
        // avoid later stages if any error was found
        if( CompilationErrors != 0 )
          throw runtime_error( "parser finished with errors" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 4: Run analyzer
        // (processes AST nodes in parser)
        if( VerboseMode )
          cout << "stage 4: running analyzer" << endl;
          
        VirconCAnalyzer Analyzer;
        Analyzer.Analyze( *Parser.ProgramAST, ProgramIsBios );
        
        // avoid later stages if any error was found
        if( CompilationErrors != 0 )
          throw runtime_error( "analyzer finished with errors" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STAGE 5: Run emitter
        // (AST nodes --> binary ROM)
        if( VerboseMode )
          cout << "stage 5: running emitter" << endl;
          
        VirconCEmitter Emitter;
        Emitter.Emit( *Parser.ProgramAST, ProgramIsBios );
        
        // avoid saving output if any error was found
        if( CompilationErrors != 0 )
          throw runtime_error( "emitter finished with errors" );
        
        // no need for debug output here (result is final)
        if( VerboseMode )
          cout << "saving output file" << endl;
        
        Emitter.SaveAssembly( OutputPath );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // on debug compilation output an additional debug info file
        if( CreateDebugVersion )
          SaveDebugInfoFile( OutputPath + ".debug", OutputPath, Parser, Emitter );
    }
    
    catch( const exception& e )
    {
        cerr << "compile: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "compilation successful" << endl;
    
    return 0;
}

