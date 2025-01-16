// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // include SDL2 headers
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
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


bool VerboseMode = false;


// =============================================================================
//      SOUND TREATMENT
// =============================================================================


// use global variables to communicate the load and save functions
int NumberOfSamples;
std::vector< uint32_t > RawSamples;

// -----------------------------------------------------------------------------

void LoadWAV( const char *WAVFilePath, int OutputRate )
{
	SDL_AudioSpec SourceAudioFormat;
	uint8_t *SourceSamples = nullptr;
	uint32_t SourceBytes = 0;
	
	// first check if the file exists
	if( !FileExists( WAVFilePath ) )
      throw runtime_error( string("cannot open input file \"") + WAVFilePath + "\"" );
    
    // load audio from the input file
    if( !SDL_LoadWAV( WAVFilePath, &SourceAudioFormat, &SourceSamples, &SourceBytes ) )
      throw runtime_error( string("failed to load file \"") + WAVFilePath + "\" as a WAV file" );
    
    // for output rate = 0 do not alter the input rate
    if( OutputRate == 0 )
    {
        OutputRate = SourceAudioFormat.freq;
        
        if( VerboseMode )
          cout << "using input sample rate of " << OutputRate << " Hz" << endl;
    }
    
    // start conversion phase
    if( VerboseMode )
      cout << "converting sound to Vircon format" << endl;
    
    // configure SDL for the audio format conversion
	SDL_AudioCVT AudioConversionInfo;
	
    SDL_BuildAudioCVT
    (
        &AudioConversionInfo,           // output structure to be filled
        SourceAudioFormat.format,       // source audio format
        SourceAudioFormat.channels,     // source channels
        SourceAudioFormat.freq,         // source frequency
        AUDIO_S16LSB,                   // destination audio format
        2,                              // destination channels
        OutputRate                      // destination frequency
    );
    
    // fill the structure's buffer with the source audio
	AudioConversionInfo.len = SourceBytes;
	AudioConversionInfo.buf = (uint8_t*)malloc( SourceBytes * AudioConversionInfo.len_mult );
	memcpy( AudioConversionInfo.buf, SourceSamples, SourceBytes );
	
    // convert input to Vircon32 format
    // (44100Hz, signed 16-bit samples, stereo)
    if( SDL_ConvertAudio( &AudioConversionInfo ) != 0 )
      throw runtime_error( string("cannot convert audio format: ") + SDL_GetError() );
	
	// we no longer need the source buffer
	SDL_FreeWAV( SourceSamples );
    
    // detect the number of samples
    int ConvertedBytes = AudioConversionInfo.len_cvt;
    NumberOfSamples = ConvertedBytes / 4;
    
    // store the converted audio samples
    RawSamples.resize( NumberOfSamples );
    memcpy( &RawSamples[0], AudioConversionInfo.buf, NumberOfSamples * 4 );
    
    // free used memory
    free( AudioConversionInfo.buf );
}

// -----------------------------------------------------------------------------

void SaveVSND( const char *VSNDFilePath )
{
    // open output file
    FILE *VSNDFile = OpenOutputFile( VSNDFilePath );
    
    if( !VSNDFile )
      throw runtime_error( string("Cannot open output file \"") + VSNDFilePath + "\"" );
    
    // create the VSND file header
    SoundFileFormat::Header VSNDHeader;
    memcpy( VSNDHeader.Signature, SoundFileFormat::Signature, 8 );
    VSNDHeader.SoundSamples = NumberOfSamples;
    
    // write the header in the file
    fseek( VSNDFile, 0, SEEK_SET );
    fwrite( &VSNDHeader, sizeof(SoundFileFormat::Header), 1, VSNDFile );
    
    // now write all samples
    fwrite( &RawSamples[0], NumberOfSamples*4, 1, VSNDFile );
    
    // close our file
    fclose( VSNDFile );
}


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: wav2vircon [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -r <rate>    Output sample rate. Default is 44100Hz (Vircon32 native)" << endl;
    cout << "               Rate = 0 means output keeps same sample rate as input" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "wav2vircon v25.1.4" << endl;
    cout << "Vircon32 WAV file importer by Javier Carracedo" << endl;
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
        int OutputRate = 44100;
        
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
            
            if( ArgumentsUTF8[i] == string("-r") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing output rate after '-r'" );
                
                // try to parse an integer from rate argument
                try
                {
                    OutputRate = stoi( ArgumentsUTF8[ i ] );
                }
                catch( const exception& e )
                {
                    throw runtime_error( "cannot read output rate as an integer" );
                }
                
                // establish some sensible boundaries
                if( (OutputRate < 1000 && OutputRate != 0) || OutputRate > 100000 )
                  throw runtime_error( "bad output rate (valid range is 1000-100000 Hz)" );
                
                continue;
            }
            
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
            OutputPath = ReplaceFileExtension( InputPath, "vsnd" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load the WAV sound
        
        // initialize SDL
        if( SDL_Init( SDL_INIT_AUDIO ) != 0 )
          throw runtime_error( string("cannot initialize SDL: ") + SDL_GetError() );
        
        // now we can actually load the wav file
        if( VerboseMode )
          cout << "loading input file \"" << InputPath << "\"" << endl;
        
        LoadWAV( InputPath.c_str(), OutputRate );
        
        // we are done with SDL
        SDL_Quit();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Save the VSND file
        
        if( VerboseMode )
          cout << "saving output file \"" << OutputPath << "\"" << endl;
        
        SaveVSND( OutputPath.c_str() );
    }
    
    catch( const exception& e )
    {
        cerr << "wav2vircon: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "conversion successful" << endl;
    
    return 0;
}
