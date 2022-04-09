// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include libpng headers
    #include <png.h>
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <string>       // [ C++ STL ] Strings
    #include <stdexcept>    // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      IMAGE TREATMENT
// =============================================================================


int ImageWidth, ImageHeight;
png_bytep *RowPointers = nullptr;

// -----------------------------------------------------------------------------

void LoadVTEX( const char *VTEXFilePath )
{
    // open input file
    FILE *VTEXFile = fopen( VTEXFilePath, "rb" );
    
    if( !VTEXFile )
      throw runtime_error( string("Cannot open intput file \"") + VTEXFilePath + "\"" );
    
    // read 2 ints for image dimensions
    fseek( VTEXFile, 0, SEEK_SET );
    
    uint32_t Width32;
    fread( &Width32, 4, 1, VTEXFile );
    ImageWidth = Width32;
    
    uint32_t Height32 = ImageHeight;
    fread( &Height32, 4, 1, VTEXFile );
    ImageHeight = Height32;
    
    // now read every row
    RowPointers = (png_bytep*)malloc(sizeof(png_bytep) * ImageHeight);
    
    for( int y = 0; y < ImageHeight; y++ )
    {
        RowPointers[y] = (png_byte*)malloc( ImageWidth*4 );
        fread( RowPointers[ y ], ImageWidth*4, 1, VTEXFile );
    }
    
    // clean-up
    fclose( VTEXFile );
}

// -----------------------------------------------------------------------------

void SavePNG( const char *PNGFilePath )
{
    // open output file
    FILE *PNGFile = fopen( PNGFilePath, "wb" );
    if( !PNGFile ) abort();

    png_structp PNGHandler = png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
    if( !PNGHandler ) abort();

    png_infop PNGInfo = png_create_info_struct( PNGHandler );
    if( !PNGInfo ) abort();

    // define a callback function expected by libpng for error handling
    if( setjmp( png_jmpbuf(PNGHandler) ) )
      abort();
      
    // begin writing
    png_init_io( PNGHandler, PNGFile );

    // define output as 8bit depth in RGBA format
    png_set_IHDR
    (
        PNGHandler,
        PNGInfo,
        ImageWidth, ImageHeight,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    
    // write basic image info
    png_write_info( PNGHandler, PNGInfo );
    
    // write the actual pixel data for all rows
    if( !RowPointers ) abort();
    png_write_image( PNGHandler, RowPointers );
    
    // end writing
    png_write_end( PNGHandler, nullptr );
    
    // clean-up
    fclose( PNGFile );
    png_destroy_write_struct( &PNGHandler, &PNGInfo );
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char* Arguments[] )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Process command line arguments
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // check number of arguments
    if( NumberOfArguments < 2 || NumberOfArguments > 3 )
    {
        cout << "USAGE: vtex2png <input path> (optional: <output path>)" << endl;
        return 0;
    }
    
    // read input path
    string InputPath, OutputPath;
    InputPath = Arguments[ 1 ];
    
    // input path must have a .vtex extension
    if( InputPath.length() < 6 )
    {
        cout << "ERROR: Input path is too short (it must include .vtex extension)" << endl;
        return 0;
    }
    
    string InputExtension = GetFileExtension( InputPath );
    
    if( ToLowerCase( InputExtension ) != "vtex" )
    {
        cout << "ERROR: Input path must have .vtex extension" << endl;
        return 0;
    }
    
    // if there is an explicit output path, read it
    if( NumberOfArguments == 3 )
    {
        OutputPath = Arguments[ 2 ];
        
        // output path must have a .png extension
        if( OutputPath.length() < 5 )
        {
            cout << "ERROR: Output path is too short (it must include .png extension)" << endl;
            return 0;
        }
        string OutputExtension = GetFileExtension( OutputPath );
        
        if( ToLowerCase( OutputExtension ) != "png" )
        {
            cout << "ERROR: Output path must have .png extension" << endl;
            return 0;
        }
    }
    
    // otherwise, use the input path but with .png extension
    else OutputPath = ReplaceFileExtension( InputPath, "png" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Load the VTEX image
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    try
    {
        LoadVTEX( InputPath.c_str() );
    }
    catch( exception& e )
    {
        cout << "Error loading VTEX file: " << e.what() << endl;
        return 1;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Save the PNG file
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    try
    {
        SavePNG( OutputPath.c_str() );
    }
    catch( exception& e )
    {
        cout << "Error saving PNG file: " << e.what() << endl;
        return 1;
    }
    
    // perform clean-up
    for( int y = 0; y < ImageHeight; y++ )
      free( RowPointers[ y ] );
    
    free( RowPointers );
}
