// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/Definitions.hpp"
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/FileSignatures.hpp"
    
    // include project headers
    #include "WavFormat.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


bool VerboseMode = false;


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void WriteChunkID( void* ChunkLocation, const char* ID )
{
    // do not use strcpy or else a 5th null byte is copied
    memcpy( ChunkLocation, ID, 4 );
}


// =============================================================================
//      SOUND TREATMENT
// =============================================================================


// use global variables to communicate the load and save functions
int NumberOfSamples;
std::vector< uint32_t > RawSamples;

// -----------------------------------------------------------------------------

void LoadVSND( const char *VSNDFilePath )
{
    // open input file
    FILE *VSNDFile = fopen( VSNDFilePath, "rb" );
    
    if( !VSNDFile )
      throw runtime_error( string("Cannot open intput file \"") + VSNDFilePath + "\"" );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    fseek( VSNDFile, 0, SEEK_END );
    unsigned FileBytes = ftell( VSNDFile );
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "Incorrect VSND file format (file size must be a multiple of 4)" );
    
    // ensure that we can at least load the file header
    if( FileBytes < sizeof(SoundFileFormat::Header) )
      throw runtime_error( "Incorrect VSND file format (file is too small)" );
    
    // load a sound file signature
    SoundFileFormat::Header VSNDHeader;
    fseek( VSNDFile, 0, SEEK_SET );
    fread( &VSNDHeader, sizeof(SoundFileFormat::Header), 1, VSNDFile );
    
    // check that it is actually a sound file
    if( !CheckSignature( VSNDHeader.Signature, SoundFileFormat::Signature ) )
      throw runtime_error( "Incorrect VSND file format (file does not have a valid signature)" );
    
    // save sound length dimensions
    NumberOfSamples = VSNDHeader.SoundSamples;
    
    // report sound length
    if( VerboseMode )
      cout << "VSND sound length is " << NumberOfSamples << " samples" << endl;
    
    // check sound size limitations
    if( !IsBetween( NumberOfSamples , 1, Constants::MaximumCartridgeProgramROM ) )
      throw runtime_error( "VSND sound does not have correct size (from 1 up to 268435456 samples)" );
    
    // check that file size matches the reported sound
    unsigned ExpectedBytes = sizeof(SoundFileFormat::Header) + 4 * NumberOfSamples;
    
    if( FileBytes != ExpectedBytes )
      throw runtime_error( "Incorrect VSND file format (file size does not match reported sound length)" );
    
    // now read every sample
    RawSamples.resize( NumberOfSamples );
    fread( &RawSamples[ 0 ], NumberOfSamples*4, 1, VSNDFile );
    
    // clean-up
    fclose( VSNDFile );
}

// -----------------------------------------------------------------------------

void SaveWAV( const char *WAVFilePath )
{
    // open output file
    FILE *WAVFile = fopen( WAVFilePath, "wb" );
    
    if( !WAVFile )
      throw runtime_error( string("Cannot open output file \"") + WAVFilePath + "\"" );
    
    // populate the RIFF header
    RIFFChunkHeader RIFFHeader;
    WriteChunkID( RIFFHeader.ChunkID, "RIFF" );
    WriteChunkID( RIFFHeader.Format, "WAVE" );
    RIFFHeader.ChunkSize = 4 + sizeof(FormatSubchunkBody) + 2*sizeof(SubchunkHeader) + 4*NumberOfSamples;
    
    // write the RIFF header
    fseek( WAVFile, 0, SEEK_SET );
    fwrite( &RIFFHeader, sizeof(RIFFChunkHeader), 1, WAVFile );
    
    // populate format header and body
    SubchunkHeader FormatHeader;
    WriteChunkID( FormatHeader.SubchunkID, "fmt " );
    FormatHeader.SubchunkSize = sizeof( FormatSubchunkBody );
    
    FormatSubchunkBody FormatBody;
    FormatBody.AudioFormat = 1;           // 1 = PCM (uncompressed)
    FormatBody.NumberOfChannels = 2;      // 2 = Stereo
    FormatBody.SampleRate = 44100;
    FormatBody.ByteRate = 44100 * 2 * 2;  // ByteRate = SampleRate * Channels * BitsPerSample/8
    FormatBody.BlockAlign = 2 * 2;        // BlockAlign = Channels * BitsPerSample/8
    FormatBody.BitsPerSample = 16;
    
    // write format header and body
    fwrite( &FormatHeader, sizeof(SubchunkHeader), 1, WAVFile );
    fwrite( &FormatBody, sizeof(FormatSubchunkBody), 1, WAVFile );
    
    // populate the data header
    SubchunkHeader DataHeader;
    WriteChunkID( DataHeader.SubchunkID, "data" );
    DataHeader.SubchunkSize = 4 * NumberOfSamples;
    
    // write the data header
    fwrite( &DataHeader, sizeof(SubchunkHeader), 1, WAVFile );
    
    // write the actual samples
    fwrite( &RawSamples[ 0 ], 4 * NumberOfSamples, 1, WAVFile );
    
    // close our file
    fclose( WAVFile );
}


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: vircon2wav [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "vircon2wav v24.8.2" << endl;
    cout << "Vircon32 WAV file extractor by Javier Carracedo" << endl;
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
            OutputPath = ReplaceFileExtension( InputPath, "wav" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load the WAV sound
        
        // now we can actually load the wav file
        if( VerboseMode )
          cout << "loading input file \"" << InputPath << "\"" << endl;
        
        LoadVSND( InputPath.c_str() );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Save the VSND file
        
        if( VerboseMode )
          cout << "saving output file \"" << OutputPath << "\"" << endl;
        
        SaveWAV( OutputPath.c_str() );
    }
    
    catch( const exception& e )
    {
        cerr << "vircon2wav: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "conversion successful" << endl;
    
    return 0;
}
