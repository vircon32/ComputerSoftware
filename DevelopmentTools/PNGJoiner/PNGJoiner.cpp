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
    #include <string.h>     // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: joinpngs [options] inputfolder outputfile" << endl;
    cout << "InputFolder: Path to a folder containing all input PNG images to join" << endl;
    cout << "OutputFile: Path for the resulting joined PNG image" << endl;
    cout << "(a region editor XML will be generated with the same name)" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -g <gap>     Separation between joined images, in pixels (default: 1)" << endl;
    cout << "  -hx <where>  Hotspot xs in XML: left/center/right (detault:left)" << endl;
    cout << "  -hy <where>  Hotspot ys in XML: top/center/bottom (detault:top)" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
    cout << endl;
    cout << "Images will be interpreted as matrices if their file name follows the" << endl;
    cout << "pattern 'name_columns_rows_gap.png'. For instance, a file with name" << endl;
    cout << "walk_4_2_1.png is taken as a grid of 4x2 images separated by 1 pixel." << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "joinpngs v24.12.29" << endl;
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
            
            if( Arguments[i] == string("-hx") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing hotspot position after '-hx'" );
                
                // read position value
                if( !strcmp( Arguments[ i ], "left" ) )
                  HotspotProportionX = 0;
                else if( !strcmp( Arguments[ i ], "center" ) )
                  HotspotProportionX = 0.5;
                else if( !strcmp( Arguments[ i ], "right" ) )
                  HotspotProportionX = 1;
                else throw runtime_error( "invalid X hotspot position (must be left, center or right)" );
                
                continue;
            }
            
            if( Arguments[i] == string("-hy") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing hotspot position after '-hy'" );
                
                // read position value
                if( !strcmp( Arguments[ i ], "top" ) )
                  HotspotProportionY = 0;
                else if( !strcmp( Arguments[ i ], "center" ) )
                  HotspotProportionY = 0.5;
                else if( !strcmp( Arguments[ i ], "bottom" ) )
                  HotspotProportionY = 1;
                else throw runtime_error( "invalid Y hotspot position (must be top, center or bottom)" );
                
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
           
            // second non-option parameter is taken as the output file
            else if( OutputFile.empty() )
            {
                OutputFile = Arguments[i];
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
        
        // check if an output file was given and is not a folder
        if( OutputFile.empty() )
          throw runtime_error( "no output file" );
        
        if( filesystem::is_directory( OutputFile ) )
          throw runtime_error( "input path must be a file, found a folder" );
      
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Scan folder and load all PNG images
        
        if( VerboseMode )
          cout << "loading PNGs from input folder \"" << InputFolder << "\"" << endl;
        
        for( auto const& DirEntry : std::filesystem::directory_iterator{ InputPath } )
        {
            // discard symlinks, non-files, etc.
            if( DirEntry.is_directory() || DirEntry.is_symlink() || !DirEntry.is_regular_file() )
              continue;
            
            // load PNG images
            if( ToLowerCase( DirEntry.path().extension().string() ) == ".png" )
            {
                LoadedImages.emplace_back();
                LoadedImages.back().LoadFromFile( DirEntry.path().string() );
            }
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Run the joining algorithm
        
        if( VerboseMode )
          cout << "running the join algorithm" << endl;
        
        // assign region IDs for all images, in initial order
        AssignRegionIDs( LoadedImages );
        
        // run the algorithm to place all images
        PlaceAllImages();
        
        /*
        for( auto& Image: LoadedImages )
        {
            cout << Image.Name << ": ID " << Image.FirstTileID <<
            " (" << Image.Width << "x" << Image.Height << " px)" <<
            " [" << Image.TilesX << "x" << Image.TilesY << "]" << endl;
        }
        */
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: Save the joined PNG file
        
        if( VerboseMode )
          cout << "saving output PNG file \"" << OutputFile << "\"" << endl;
        
        /*
        for( auto& Image: LoadedImages )
            Image.SaveToFile(InputFolder + "/out/" + Image.Name + ".png");
        */
        
        // create an empty image to hold all subimages
        int MaxUsedX, MaxUsedY;
        TextureRectangle.GetContentsLimit( MaxUsedX, MaxUsedY );
        
        PNGImage TextureImage;
        TextureImage.CreateEmpty( MaxUsedX+1, MaxUsedY+1 );
        
        // copy all subimages into the output image and save it
        CreateOutputImage( TextureImage, TextureRectangle );
        TextureImage.SaveToFile( OutputFile );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Create texture region editor project for the texture
        
        if( VerboseMode )
          cout << "creating region editor project for the joined image" << endl;
        
        SaveRegionEditorProject( ReplaceFileExtension( OutputFile, "xml" ) );
    }
    
    catch( const exception& e )
    {
        cerr << "joinpngs: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "images joined successfully" << endl;
    
    return 0;
}
