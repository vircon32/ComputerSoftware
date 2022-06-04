// *****************************************************************************
    // include Vircon common headers
    #include "../../VirconDefinitions/VirconROMFormat.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include libpng headers
    #include <png.h>
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <string>       // [ C++ STL ] Strings
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <cstring>      // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


bool Debug = false;
bool VerboseMode = false;


// =============================================================================
//      IMAGE TREATMENT
// =============================================================================


// use global variables to communicate the load and save functions
int ImageWidth = 0, ImageHeight = 0;
png_byte ColorType;
png_byte BitDepth;
png_bytep *RowPointers = nullptr;

// -----------------------------------------------------------------------------

void LoadPNG( const char *PNGFilePath )
{
    // open input file
    FILE *PNGFile = fopen( PNGFilePath, "rb" );
    
    if( !PNGFile )
      throw runtime_error( string("cannot open input file \"") + PNGFilePath + "\"" );
    
    // allocate structures to hold PNG information
    png_structp PNGHandler = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
    
    if(!PNGHandler)
      throw runtime_error( "cannot allocate PNG handler structure" );

    png_infop PNGInfo = png_create_info_struct( PNGHandler );
    
    if(!PNGInfo)
      throw runtime_error( "cannot allocate PNG information structure" );
    
    // define a callback function expected by libpng for error handling
    if( setjmp( png_jmpbuf( PNGHandler ) ) )
      throw runtime_error( "cannot define an error callback for PNG loading functions" );
    
    // read basic information into the structures we created
    png_init_io( PNGHandler, PNGFile );
    png_read_info( PNGHandler, PNGInfo );
    
    // extract their basic fields using the premade functions
    ImageWidth  = png_get_image_width ( PNGHandler, PNGInfo );
    ImageHeight = png_get_image_height( PNGHandler, PNGInfo );
    ColorType   = png_get_color_type  ( PNGHandler, PNGInfo );
    BitDepth    = png_get_bit_depth   ( PNGHandler, PNGInfo );
    
    // start conversion phase
    if( VerboseMode )
      cout << "converting image to Vircon format" << endl;
    
    // Read any ColorType into 8bit depth, VTEX format
    if( BitDepth == 16 )
      png_set_strip_16( PNGHandler );
    
    if( ColorType == PNG_COLOR_TYPE_PALETTE )
      png_set_palette_to_rgb( PNGHandler );

    // grayscale images might have less than 8 bit color depth
    if( ColorType == PNG_COLOR_TYPE_GRAY && BitDepth < 8 )
      png_set_expand_gray_1_2_4_to_8( PNGHandler );
    
    if( png_get_valid(PNGHandler, PNGInfo, PNG_INFO_tRNS ))
      png_set_tRNS_to_alpha( PNGHandler );
    
    // if there is no alpha channel, create it as totally opaque
    if( ColorType == PNG_COLOR_TYPE_RGB
    ||  ColorType == PNG_COLOR_TYPE_GRAY
    ||  ColorType == PNG_COLOR_TYPE_PALETTE )
      png_set_filler( PNGHandler, 0xFF, PNG_FILLER_AFTER );
    
    // for grayscale images, convert grays to RGB color
    if( ColorType == PNG_COLOR_TYPE_GRAY
    ||  ColorType == PNG_COLOR_TYPE_GRAY_ALPHA )
      png_set_gray_to_rgb( PNGHandler );
    
    png_read_update_info( PNGHandler, PNGInfo );
    
    // save pointers for each row in the image
    RowPointers = (png_bytep*)malloc( sizeof(png_bytep) * ImageHeight );
    
    for( int y = 0; y < ImageHeight; y++ )
      RowPointers[y] = (png_byte*)malloc( png_get_rowbytes(PNGHandler, PNGInfo) );
    
    // now read the actual pixel data for each row??
    png_read_image( PNGHandler, RowPointers );

    // clean-up
    fclose( PNGFile );
    png_destroy_read_struct( &PNGHandler, &PNGInfo, nullptr );
}

// -----------------------------------------------------------------------------

void SaveVTEX( const char *VTEXFilePath )
{
    // open output file
    FILE *VTEXFile = fopen( VTEXFilePath, "wb" );
    
    if( !VTEXFile )
      throw runtime_error( string("cannot open output file \"") + VTEXFilePath + "\"" );
    
    // create the VTEX file header
    TextureFileHeader VTEXHeader;
    memcpy( VTEXHeader.Signature, Signatures::TextureFile, 8 );
    VTEXHeader.TextureWidth = ImageWidth;
    VTEXHeader.TextureHeight = ImageHeight;
    
    // write the header in the file
    fseek( VTEXFile, 0, SEEK_SET );
    fwrite( &VTEXHeader, sizeof(TextureFileHeader), 1, VTEXFile );
    
    // now write every row of pixels
    for( int y = 0; y < ImageHeight; y++ )
      fwrite( RowPointers[ y ], ImageWidth*4, 1, VTEXFile );
    
    // clean-up
    fclose( VTEXFile );
}


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: png2vircon [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <file>    Output file, default name is the same as input" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "png2vircon v22.6.4" << endl;
    cout << "Vircon32 PNG file importer by Javier Carracedo" << endl;
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
            OutputPath = ReplaceFileExtension( InputPath, "vtex" );
            
            if( VerboseMode )
              cout << "using output path: \"" << OutputPath << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load the PNG image
        
        if( VerboseMode )
          cout << "loading input file \"" << InputPath << "\"" << endl;
        
        LoadPNG( InputPath.c_str() );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Save the VTEX file
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        if( VerboseMode )
          cout << "saving output file \"" << OutputPath << "\"" << endl;
        
        SaveVTEX( OutputPath.c_str() );
    }
    
    catch( const exception& e )
    {
        cerr << "png2vircon: error: " << e.what() << endl;
        return 1;
    }
    
    // perform clean-up
    for( int y = 0; y < ImageHeight; y++ )
      free( RowPointers[ y ] );
    
    free( RowPointers );
    
    // report success
    if( VerboseMode )
      cout << "conversion successful" << endl;
    
    return 0;
}
