// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "PNGImage.hpp"
    #include "RectangleNode.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <filesystem>   // [ C++ STL ] File system
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <string>       // [ C++ STL ] Strings
    #include <stdexcept>    // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: joinpngs [options] inputfolder outputfile" << endl;
    cout << "InputFolder: Path to a folder containing all input PNG images to join:" << endl;
    cout << "OutputFile: Path for the resulting joined PNG image" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -g <gap>     Separation between joined images, in pixels (default: 0)" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "joinpngs v24.11.4" << endl;
    cout << "Vircon32 PNG image joiner by Javier Carracedo" << endl;
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
        string InputFolder, OutputFile;
        
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
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing gap after '-g'" );
                
                // try to parse an integer from rate argument
                try
                {
                    GapBetweenImages = stoi( Arguments[ i ] );
                }
                catch( const exception& e )
                {
                    throw runtime_error( "cannot read gap as an integer" );
                }
                
                // establish some sensible boundaries
                if( GapBetweenImages < 0 || GapBetweenImages > 10 )
                  throw runtime_error( "bad gap value (valid range is 0-10 pixels)" );
                
                continue;
            }
            
            // discard any other parameters starting with '-'
            if( Arguments[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + Arguments[i] + "'" );
            
            // first non-option parameter is taken as the input folder
            if( InputFolder.empty() )
            {
                InputFolder = Arguments[i];
            }
            
            // only a single input file is supported!
            else
              throw runtime_error( "too many input files, only 1 is supported" );
        }
        
        // check if an input folder was given
        if( InputFolder.empty() )
          throw runtime_error( "no input folder" );
        
        // check that it exists and is a folder
        filesystem::path InputPath( InputFolder );
        
        if( !filesystem::exists( InputPath ) )
          throw runtime_error( "input folder does not exist" );
        
        if( !filesystem::is_directory( InputPath ) )
          throw runtime_error( "input path is a file, expected a folder" );
        
        // check if an output file was given
        if( OutputFile.empty() )
          throw runtime_error( "no output file" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Scan folder and load all PNG images
        
        if( VerboseMode )
          cout << "loading PNGs from input folder \"" << InputFolder << "\"" << endl;
        
        for( auto const& DirEntry : std::filesystem::directory_iterator{ InputPath } )
        {
            // discard symlinks, non-files, etc.
            //
            
            if( ToLowerCase( DirEntry.path().extension().string() ) == ".png" )
            {
                LoadedImages.emplace_back();
                LoadedImages.back().LoadFromFile( DirEntry.path().string() );
            }
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Run the joining algorithm
        
        // first sort all images
        LoadedImages.sort();
        
        // initialize tree root
        //
        
        // run the algorithm
        //
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: Save the joined PNG file
        
        if( VerboseMode )
          cout << "saving output file \"" << OutputFile << "\"" << endl;
        
        PNGImage TextureImage;
        // ...
        TextureImage.SaveToFile( OutputFile );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Create C header and source for the texture
        
        if( VerboseMode )
          cout << "saving C header and source for the texture" << endl;
        
        SaveCTextureHeader( ReplaceFileExtension( OutputFile, "h" ) );
        SaveCTextureSource( ReplaceFileExtension( OutputFile, "c" ) );
    }
    
    catch( const exception& e )
    {
        cerr << "joinpngs: error: " << e.what() << endl;
        return 1;
    }
    
    // perform clean-up
    // (none?? check)
    
    // report success
    if( VerboseMode )
      cout << "images joined successfully" << endl;
    
    return 0;
}
