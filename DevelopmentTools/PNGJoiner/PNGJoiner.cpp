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
    #include <vector>       // [ C++ STL ] Vectors
    #include <string.h>     // [ ANSI C ] Strings
    
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
    cout << "The matrix should not have any surrounding border." << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "joinpngs v25.10.29" << endl;
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
            
            if( ArgumentsUTF8[i] == string("-g") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing gap after '-g'" );
                
                // try to parse an integer from rate argument
                try
                {
                    GapBetweenImages = stoi( ArgumentsUTF8[ i ] );
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
            
            if( ArgumentsUTF8[i] == string("-hx") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing hotspot position after '-hx'" );
                
                // read position value
                if( ArgumentsUTF8[ i ] == string("left") )
                  HotspotProportionX = 0;
                else if( ArgumentsUTF8[ i ] == string("center") )
                  HotspotProportionX = 0.5;
                else if( ArgumentsUTF8[ i ] == string("right") )
                  HotspotProportionX = 1;
                else throw runtime_error( "invalid X hotspot position (must be left, center or right)" );
                
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-hy") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing hotspot position after '-hy'" );
                
                // read position value
                if( ArgumentsUTF8[ i ] == string("top") )
                  HotspotProportionY = 0;
                else if( ArgumentsUTF8[ i ] == string("center") )
                  HotspotProportionY = 0.5;
                else if( ArgumentsUTF8[ i ] == string("bottom") )
                  HotspotProportionY = 1;
                else throw runtime_error( "invalid Y hotspot position (must be top, center or bottom)" );
                
                continue;
            }
            
            // discard any other parameters starting with '-'
            if( ArgumentsUTF8[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + ArgumentsUTF8[i] + "'" );
            
            // first non-option parameter is taken as the input folder
            if( InputFolder.empty() )
            {
                InputFolder = ArgumentsUTF8[i];
            }
           
            // second non-option parameter is taken as the output file
            else if( OutputFile.empty() )
            {
                OutputFile = ArgumentsUTF8[i];
            }
            
            // only a single input file is supported!
            else
              throw runtime_error( "too many arguments" );
        }
        
        // check if an input folder was given
        if( InputFolder.empty() )
          throw runtime_error( "no input folder" );
        
        // check that it exists and is a folder
        filesystem::path InputFolderPath = filesystem::u8path( InputFolder );
        
        if( !filesystem::exists( InputFolderPath ) )
          throw runtime_error( "input folder does not exist" );
        
        if( !filesystem::is_directory( InputFolderPath ) )
          throw runtime_error( "input path is a file, expected a folder" );
        
        // check if an output file was given and is not a folder
        if( OutputFile.empty() )
          throw runtime_error( "no output file" );
        
        filesystem::path OutputFilePath = filesystem::u8path( OutputFile );
        
        if( filesystem::is_directory( OutputFilePath ) )
          throw runtime_error( "input path must be a file, found a folder" );
      
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Scan folder and load all PNG images
        
        if( VerboseMode )
          cout << "loading PNGs from input folder \"" << InputFolder << "\"" << endl;
        
        for( auto const& DirEntry : std::filesystem::directory_iterator{ InputFolderPath } )
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
        
        // keep a list of pointers with the original order
        // since we want to preserve that order on output
        for( auto& Image: LoadedImages )
          SortedImages.push_back( &Image );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Run the joining algorithm
        
        if( VerboseMode )
          cout << "running the join algorithm" << endl;
        
        // extend texture with the separation gap
        // at bottom and right, so that the actual
        // usable area is still the same
        TextureRectangle.MaxX += GapBetweenImages;
        TextureRectangle.MaxY += GapBetweenImages;
        
        // run the algorithm to place all images
        PlaceAllImages();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: Save the joined PNG file
        
        if( VerboseMode )
          cout << "saving output PNG file \"" << OutputFile << "\"" << endl;
        
        // create an empty image to hold all subimages
        int MaxUsedX, MaxUsedY;
        TextureRectangle.GetContentsLimit( MaxUsedX, MaxUsedY );
        
        PNGImage TextureImage;
        TextureImage.CreateEmpty( (MaxUsedX+1) - GapBetweenImages, (MaxUsedY+1) - GapBetweenImages );
        
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
