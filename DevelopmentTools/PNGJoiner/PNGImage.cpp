// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "PNGImage.hpp"
    #include "Globals.hpp"
    
    // include libpng headers
    #include <png.h>
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <sstream>      // [ C++ STL ] String streams
    #include <vector>       // [ C++ STL ] Vectors
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <algorithm>    // [ C++ STL ] Algorithms
    #include <cstring>      // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PNG IMAGE CLASS
// =============================================================================


PNGImage::PNGImage()
{
    Width = Height = 0;
    FirstTileID = 0;
    TilesX = TilesY = 1;
    TilesGap = 0;
    RowPixels = nullptr;
}

// -----------------------------------------------------------------------------

PNGImage::PNGImage( const PNGImage& Copied )
{
    // copy all regular fields
    Name = Copied.Name;
    FirstTileID = Copied.FirstTileID;
    Width = Copied.Width;
    Height = Copied.Height;
    TilesX = Copied.TilesX;
    TilesY = Copied.TilesY;
    TilesGap = Copied.TilesGap;
    
    // recursively copy children
    RowPixels = nullptr;
    
    if( Copied.RowPixels )
    {
        RowPixels = (uint8_t**)malloc( sizeof(uint8_t*) * Height );
        
        for( int y = 0; y < Height; y++ )
        {
            RowPixels[ y ] = (uint8_t*)malloc( 4 * Width * sizeof(uint8_t*) );
            memcpy( Copied.RowPixels[ y ], RowPixels[ y ], 4 * Width * sizeof(uint8_t*) );
        }
    }
}

// -----------------------------------------------------------------------------

PNGImage::~PNGImage()
{
    // free image pixels if any
    for( int y = 0; y < Height; y++ )
      free( RowPixels[ y ] );
    
    free( RowPixels );
    RowPixels = nullptr;
}

// -----------------------------------------------------------------------------

// auxiliary function to decompose names
// (throws if not a valid number)
int ExtractNumber( const std::string& Text )
{
    int Number = stoi( Text );
    
    if( Number < 0 )
      throw runtime_error( "" );
    
    return Number;
}

// -----------------------------------------------------------------------------

void PNGImage::DecomposeFileName( const string& PNGFilePath )
{
    // remove folder and extension
    string FileName = GetPathFileName( PNGFilePath );
    FileName = GetFileWithoutExtension( FileName );
    
    // split by underscores
    stringstream SStream( FileName );
    vector< string > NameParts;
    string Part;
    
    while( getline( SStream, Part, '_' ) )
      NameParts.push_back( Part );
    
    // check we have at least 4 parts
    // (name, columns, rows, gap)
    if( NameParts.size() < 4 )
    {
        Name = FileName;
        TilesX = TilesY = 1;
        TilesGap = 0;
        return;
    }
    
    // check that last 3 parts are numbers
    try
    {
        // read and set their fields
        TilesX   = ExtractNumber( NameParts[ NameParts.size() - 3 ] );
        TilesY   = ExtractNumber( NameParts[ NameParts.size() - 2 ] );
        TilesGap = ExtractNumber( NameParts[ NameParts.size() - 1 ] );
        
        // set the rest as the name
        Name = NameParts[ 0 ];
        
        for( unsigned i = 1; i < (NameParts.size() - 3); i++ )
          Name += "_" + NameParts[ i ];
    }
    catch( exception& e )
    {
        Name = FileName;
        TilesX = TilesY = 1;
        TilesGap = 0;
    }
}

// -----------------------------------------------------------------------------

void PNGImage::LoadFromFile( const string& PNGFilePath )
{
    DecomposeFileName( PNGFilePath );
    
    // replace any spaces in the name with underscores
    replace( Name.begin(), Name.end(), ' ', '_' );
    
    // matrix dimensions cannot be zero
    string FileName = GetPathFileName( PNGFilePath );
    
    if( TilesX <= 0 || TilesY <= 0 )
      throw runtime_error( "File \"" + FileName + "\" states non-positive matrix dimensions" );
    
    // open input file
    FILE *PNGFile = fopen( PNGFilePath.c_str(), "rb" );
    
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
    Width  = png_get_image_width ( PNGHandler, PNGInfo );
    Height = png_get_image_height( PNGHandler, PNGInfo );
    
    uint8_t ColorType = png_get_color_type( PNGHandler, PNGInfo );
    uint8_t BitDepth  = png_get_bit_depth ( PNGHandler, PNGInfo );
    
    // convert any color type and depth into 8-bit RGB
    if( BitDepth == 16 )
      png_set_strip_16( PNGHandler );
    
    if( ColorType == PNG_COLOR_TYPE_PALETTE )
      png_set_palette_to_rgb( PNGHandler );

    // grayscale images might have less than 8 bit color depth
    if( ColorType == PNG_COLOR_TYPE_GRAY && BitDepth < 8 )
      png_set_expand_gray_1_2_4_to_8( PNGHandler );
    
    if( png_get_valid( PNGHandler, PNGInfo, PNG_INFO_tRNS ))
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
    RowPixels = (uint8_t**)malloc( sizeof(uint8_t*) * Height );
    
    for( int y = 0; y < Height; y++ )
      RowPixels[ y ] = (uint8_t*)malloc( png_get_rowbytes(PNGHandler, PNGInfo) );
    
    // now read the actual pixel data for each row??
    png_read_image( PNGHandler, RowPixels );

    // clean-up
    fclose( PNGFile );
    png_destroy_read_struct( &PNGHandler, &PNGInfo, nullptr );
    
    // images can't be larger than a Vircon32 texture
    if( Width > 1024 || Height > 1024 )
      throw runtime_error( "Image in file \"" + FileName + "\" is too large (limit is 1024 pixels)" );
    
    // for matrices, check that their dimensions match the stated
    if( TilesX > 1 )
    {
        int EffectiveWidth = Width - TilesGap * (TilesX-1);
        
        if( EffectiveWidth < TilesX || (EffectiveWidth % TilesX) != 0)
          throw runtime_error( "Image width of file \"" + FileName + "\" does not match the stated matrix" );
    }
    
    if( TilesY > 1 )
    {
        int EffectiveHeight = Height - TilesGap * (TilesY-1);
        
        if( EffectiveHeight < TilesY || (EffectiveHeight % TilesY) != 0)
          throw runtime_error( "Image height of file \"" + FileName + "\" does not match the stated matrix" );
    }
}


// -----------------------------------------------------------------------------

void PNGImage::SaveToFile( const string& PNGFilePath ) const
{
    // open output file
    FILE *PNGFile = fopen( PNGFilePath.c_str(), "wb" );
    
    if( !PNGFile )
      throw runtime_error( "Cannot open output file for writing" );

    png_structp PNGHandler = png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
    
    if( !PNGHandler )
      throw runtime_error( "Cannot create a PNG handler" );

    png_infop PNGInfo = png_create_info_struct( PNGHandler );
    
    if( !PNGInfo )
      throw runtime_error( "Cannot create a PNG info structure" );

    // define a callback function expected by libpng for error handling
    if( setjmp( png_jmpbuf(PNGHandler) ) )
      throw runtime_error( "Cannot set callback function for libpng" );
      
    // begin writing
    png_init_io( PNGHandler, PNGFile );

    // define output as 8bit depth in RGBA format
    png_set_IHDR
    (
        PNGHandler,
        PNGInfo,
        Width, Height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    
    // write basic image info
    png_write_info( PNGHandler, PNGInfo );
    
    // write the actual pixel data for all rows
    if( !RowPixels )
      throw runtime_error( "The row pointers are not allocated" );
    
    png_write_image( PNGHandler, RowPixels );
    
    // end writing
    png_write_end( PNGHandler, nullptr );
    
    // clean-up
    fclose( PNGFile );
    png_destroy_write_struct( &PNGHandler, &PNGInfo );
}

// -----------------------------------------------------------------------------

void PNGImage::CreateEmpty( int NewWidth, int NewHeight )
{
    // fill basic parameters
    Width = NewWidth;
    Height = NewHeight;
    TilesX = TilesY = 1;
    TilesGap = 0;
    
    // create the pointers
    RowPixels = (uint8_t**)malloc( sizeof(uint8_t*) * NewHeight );
    
    // create rows and set all to 0 (transparent pixels)
    for( int y = 0; y < NewHeight; y++ )
      RowPixels[ y ] = (uint8_t*)calloc( 4 * NewWidth, sizeof(uint8_t) );
}

// -----------------------------------------------------------------------------

void PNGImage::CopySubImage( const PNGImage& SubImage, int LeftX, int TopY )
{
    // check that subimage will be fully inside this image
    if( LeftX < 0 || (LeftX+SubImage.Width-1) >= Width )
      throw runtime_error( "Can't place subimage inside image" );
  
    if( TopY < 0 || (TopY+SubImage.Height-1) >= Height )
      throw runtime_error( "Can't place subimage inside image" );
    
    // copy pixels for each row
    for( int y = 0; y < SubImage.Height; y++ )
      memcpy( &RowPixels[ TopY + y ][ LeftX * 4 ], SubImage.RowPixels[ y ], SubImage.Width * 4 * sizeof(uint8_t) );
}

// -----------------------------------------------------------------------------

int PNGImage::Area() const
{
    return Width * Height;
}

// -----------------------------------------------------------------------------

int PNGImage::PaddedWidth() const
{
    return Width + GapBetweenImages;
}

// -----------------------------------------------------------------------------

int PNGImage::PaddedHeight() const
{
    return Height + GapBetweenImages;
}

// -----------------------------------------------------------------------------

int PNGImage::PaddedArea() const
{
    return PaddedWidth() * PaddedHeight();
}


// =============================================================================
//      SORTING IMAGES
// =============================================================================


bool operator<( const PNGImage& Image1, const PNGImage& Image2 )
{
    // main priority is sorting from widest to thinnest
    if( Image1.Width != Image2.Width )
      return (Image1.Width > Image2.Width);
    
    // for equal width the largest image goes first
    if( Image1.Height != Image2.Height )
      return (Image1.Height > Image2.Height);
    
    // otherwise keep the initial order
    return (Image1.FirstTileID < Image2.FirstTileID);
}

// -----------------------------------------------------------------------------

void AssignRegionIDs( std::list< PNGImage >& LoadedImages )
{
    int NextRegionID = 0;
    
    for( auto& Image: LoadedImages )
    {
        Image.FirstTileID = NextRegionID;
        NextRegionID += Image.TilesX * Image.TilesY;
    }
}
