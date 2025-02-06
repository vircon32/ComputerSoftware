// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "Texture.hpp"
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      TEXTURE: INSTANCE HANDLING
// =============================================================================


Texture::Texture()
// - - - - - - - - - - - -
:   TextureID    ( 0 ),
    TextureWidth ( 0 ),
    TextureHeight( 0 ),
    ImageWidth   ( 0 ),
    ImageHeight  ( 0 ),
    HotSpotX     ( 0 ),
    HotSpotY     ( 0 )
// - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

Texture::~Texture()
{
    if( TextureID )
      Release();
}


// =============================================================================
//      TEXTURE: RESOURCE HANDLING
// =============================================================================


void Texture::Load( const string& FileName )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: USE SDL_IMAGE TO LOAD IMAGE FROM FILE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    LOG( "Texture -> Load \"" + FileName + "\"" );
    SDL_Surface* LoadedImage = NULL;
    
    // load image from file
    LoadedImage = SDL_LoadBMP( FileName.c_str() );
    
    // check errors
    if( !LoadedImage )
      THROW( "Could not load image into an SDL surface" );
    
    // read image dimensions
    ImageWidth  = LoadedImage->w;
    ImageHeight = LoadedImage->h;
    
    // read image dimensions
    TextureWidth  = NextPowerOf2( ImageWidth  );
    TextureHeight = NextPowerOf2( ImageHeight );
    
    // convert surface to 32-bit RGBA
    SDL_Surface* Aux = LoadedImage;
    SDL_Surface* ConvertedImage = SDL_ConvertSurfaceFormat( LoadedImage, SDL_PIXELFORMAT_RGBA32, 0 );
    
    if( !ConvertedImage )
      THROW( "SDL failed to convert texture to RGBA" );
    
    LoadedImage = ConvertedImage;
    SDL_FreeSurface( Aux );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: CREATE OPENGL TEXTURE FROM LOADED IMAGE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // create a new OpenGL texture and select it
    glGenTextures( 1, &TextureID );
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // check correct texture ID
    if( !TextureID )
      THROW( "OpenGL failed to generate a new texture" );
    
    // clear OpenGL errors
    glGetError();
    
    // (1) first we build an empty texture of the extented size
    glTexImage2D
    (
        GL_TEXTURE_2D,          // texture is a 2D rectangle
        0,                      // level of detail (0 = normal size)
        GL_RGBA,                // color components in the texture
        TextureWidth,           // texture width in pixels
        TextureHeight,          // texture height in pixels
        0,                      // border width (must be 0 or 1)
        GL_RGBA,                // color components in the source
        GL_UNSIGNED_BYTE,       // each color component is a byte
        nullptr                 // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create an empty OpenGL texture" );
    
    // (2) then we modify the part of our image
    glTexSubImage2D
    (   
        GL_TEXTURE_2D,          // texture is a 2D rectangle
        0,                      // level of detail (0 = normal size)
        0,                      // x offset
        0,                      // y offset
        ImageWidth,             // image width in pixels
        ImageHeight,            // image height in pixels
        GL_RGBA,                // color components in the source
        GL_UNSIGNED_BYTE,       // each color component is a byte
        LoadedImage->pixels     // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not copy the loaded SDL image to the OpenGL texture" );
    
    // delete the original SDL surface loaded by SDL_Image
    SDL_FreeSurface( LoadedImage );
    
    // for this console none of our textures will be smoothed
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // configure texture edges to NOT wrap (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    // default hotspot placement is at the center
    HotSpotX = ImageWidth / 2;
    HotSpotY = ImageHeight / 2;
    
    // save source image path to identify this texture
    LoadedFile = FileName;
}

// -----------------------------------------------------------------------------

void Texture::Release()
{
    if( !TextureID ) return;
    
    // delete the OpenGL texture
    LOG( "Texture -> Release \"" + LoadedFile + "\"" );
    glDeleteTextures( 1, &TextureID );
    TextureID = 0;
}


// =============================================================================
//      TEXTURE: DRAWING ON SCREEN (WHOLE TEXTURE)
// =============================================================================


void Texture::Draw( int HotSpotPositionX, int HotSpotPositionY ) const
{
    // check that there is a texture
    if( !TextureID )
      return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // precalculate limit coordinates
    float RenderXMin = HotSpotPositionX - HotSpotX;
    float RenderYMin = HotSpotPositionY - HotSpotY;
    
    float RenderXMax = RenderXMin + ImageWidth;
    float RenderYMax = RenderYMin + ImageHeight;    
    
    // call the coordinate-based function
    Draw( RenderXMin, RenderYMin, RenderXMax, RenderYMax );
}

// -----------------------------------------------------------------------------

void Texture::Draw( int RenderXMin, int RenderYMin, int RenderXMax, int RenderYMax ) const
{
    // check that there is a texture
    if( !TextureID )
      return;
    
    // we must render any pending quads before
    // applying any new render configurations
    Video.RenderQuadQueue();
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // calculate proportions of the image within the texture
    float XFactor = (float)ImageWidth/TextureWidth;
    float YFactor = (float)ImageHeight/TextureHeight;
    
    // build a quad to draw the texture
    GPUQuad DrawnQuad =
    {
        {
            // 4x (vertex position + texture coordinates)
            { (float)RenderXMin, (float)RenderYMin,     0.0,     0.0 },
            { (float)RenderXMax, (float)RenderYMin, XFactor,     0.0 },
            { (float)RenderXMin, (float)RenderYMax,     0.0, YFactor },
            { (float)RenderXMax, (float)RenderYMax, XFactor, YFactor }
        }
    };
    
    // draw rectangle defined as a separate quad,
    // since we use different render configuration
    Video.AddQuadToQueue( DrawnQuad );
    Video.RenderQuadQueue();
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}
