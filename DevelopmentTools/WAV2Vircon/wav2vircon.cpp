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

void LoadWAV( const char *WAVFilePath )
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
    
    // detect the number of samples
    int BytesPerSample = SDL_AUDIO_BITSIZE( SourceAudioFormat.format ) * SourceAudioFormat.channels / 8;
    NumberOfSamples = SourceBytes / BytesPerSample;
    
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
        44100                           // destination frequency
    );
    
    // fill the structure's buffer with the source audio
	AudioConversionInfo.len = SourceBytes;
	AudioConversionInfo.buf = (uint8_t*)malloc( SourceBytes * AudioConversionInfo.len_mult );
	memcpy( AudioConversionInfo.buf, SourceSamples, SourceBytes );
	
    // convert input to Vircon format
    // (44100Hz, signed 16-bit samples, stereo)
	SDL_ConvertAudio( &AudioConversionInfo );
	
	// we no longer need the source buffer
	SDL_FreeWAV( SourceSamples );
    
    // store the converted audio samples
    RawSamples.resize( NumberOfSamples );
    memcpy( &RawSamples[0], AudioConversionInfo.buf, NumberOfSamples * 4 );
}

// -----------------------------------------------------------------------------

void SaveVSND( const char *VSNDFilePath )
{
    // open output file
    FILE *VSNDFile = fopen( VSNDFilePath, "wb" );
    
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
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "wav2vircon v24.8.4" << endl;
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
        
        LoadWAV( InputPath.c_str() );
        
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
