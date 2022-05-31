// *****************************************************************************
    // include project headers
    #include "Definitions.hpp"
    #include "LogStream.hpp"
    #include "Texture.hpp"
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
    #include <SDL2/SDL_image.h>     // [ SDL2 ] SDL_Image
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      TEXTURE: INSTANCE HANDLING
// =============================================================================


Texture::Texture()
// - - - - - - - - - - - -
:   OpenGL2D (nullptr),
    TextureID    ( 0 ),
    TextureWidth ( 0 ),
    TextureHeight( 0 ),
    ImageWidth   ( 0 ),
    ImageHeight  ( 0 ),
    PixelWidth   ( 1 ),
    PixelHeight  ( 1 ),
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


void Texture::SetOpenGLContext( OpenGL2DContext& OpenGL2D_ )
{
    // capture the owner OpenGL context
    OpenGL2D = &OpenGL2D_;
}

// -----------------------------------------------------------------------------

void Texture::Load( const string& FileName )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: USE SDL_IMAGE TO LOAD IMAGE FROM FILE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    LOG( "Texture -> Load \"" << FileName << "\"" );
    SDL_Surface* LoadedImage = NULL;
    
    // load image from file
    LoadedImage = IMG_Load( FileName.c_str() );
    
    // check errors
    if( !LoadedImage )
      THROW( "Could not load image into an SDL surface" );
    
    // only 32bpp images are supported
    if( LoadedImage->format->BitsPerPixel < 24 )
      THROW( "Only true color images are supported (24 or 32 bits per pixel)" );
    
    // read image dimensions
    ImageWidth  = LoadedImage->w;
    ImageHeight = LoadedImage->h;
    
    // read image dimensions
    TextureWidth  = NextPowerOf2( ImageWidth  );
    TextureHeight = NextPowerOf2( ImageHeight );
    
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
    
    // format configuration to build the OpenGL texture from the SDL surface
    GLenum ImageType = (LoadedImage->format->BytesPerPixel == 4)? GL_RGBA : GL_RGB;
    
    // (1) first we build an empty texture of the extented size
    glTexImage2D
    (
        GL_TEXTURE_2D,          // texture is a 2D rectangle
        0,                      // level of detail (0 = normal size)
        GL_RGBA,                // color components in the texture
        TextureWidth,           // texture width in pixels
        TextureHeight,          // texture height in pixels
        0,                      // border width (must be 0 or 1)
        ImageType,              // color components in the source
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
        ImageType,              // color components in the source
        GL_UNSIGNED_BYTE,       // each color component is a byte
        LoadedImage->pixels     // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not copy the loaded SDL image to the OpenGL texture" );
    
    // delete the original SDL surface loaded by SDL_Image
    delete LoadedImage;
    
    // for this console none of our textures will be smoothed
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // configure texture edges to NOT wrap (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    // default hotspot placement is at the center
    HotSpotX = ImageWidth / 2;
    HotSpotY = ImageHeight / 2;
    
    LoadedFile = FileName;
}

// -----------------------------------------------------------------------------

void Texture::Release()
{
    if( !TextureID ) return;
    
    // delete the OpenGL texture
    LOG( "Texture -> Release \"" << LoadedFile << "\"" );
    glDeleteTextures( 1, &TextureID );
    TextureID = 0;
    
    // remove the OpenGL context
    OpenGL2D = nullptr;
}


// =============================================================================
//      TEXTURE: DRAWING ON SCREEN (WHOLE TEXTURE)
// =============================================================================


void Texture::Draw( const Vector2D& HotSpotPosition ) const
{
    // check that there is a texture and a context
    if( !TextureID || !OpenGL2D )
      return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // calculate texture render size
    float RenderWidth  = PixelWidth  * ImageWidth;
    float RenderHeight = PixelHeight * ImageHeight;
    
    // precalculate limit coordinates
    float RenderXMin = HotSpotPosition.x - HotSpotX;
    float RenderYMin = HotSpotPosition.y - HotSpotY;
    
    float RenderXMax = RenderXMin + RenderWidth;
    float RenderYMax = RenderYMin + RenderHeight;    
    
    // call the coordinate-based function
    Draw( RenderXMin, RenderYMin, RenderXMax, RenderYMax );
}

// -----------------------------------------------------------------------------

void Texture::Draw( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const
{
    // check that there is a texture and a context
    if( !TextureID || !OpenGL2D )
      return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // calculate proportions of the image within the texture
    float XFactor = (float)ImageWidth/TextureWidth;
    float YFactor = (float)ImageHeight/TextureHeight;
    
    // to avoid distortion, round screen coordinates to integers
    int RenderXMinInt = round( RenderXMin );
    int RenderYMinInt = round( RenderYMin );
    int RenderXMaxInt = round( RenderXMax );
    int RenderYMaxInt = round( RenderYMax );
    
    // set vertex positions (in render coordinates)
    OpenGL2D->SetQuadVertexPosition( 0, RenderXMinInt, RenderYMinInt );
    OpenGL2D->SetQuadVertexPosition( 1, RenderXMaxInt, RenderYMinInt );
    OpenGL2D->SetQuadVertexPosition( 2, RenderXMinInt, RenderYMaxInt );
    OpenGL2D->SetQuadVertexPosition( 3, RenderXMaxInt, RenderYMaxInt );
    
    // and texture coordinates (relative to texture: [0-1])
    OpenGL2D->SetQuadVertexTexCoords( 0,     0.0,     0.0 );
    OpenGL2D->SetQuadVertexTexCoords( 1, XFactor,     0.0 );
    OpenGL2D->SetQuadVertexTexCoords( 2,     0.0, YFactor );
    OpenGL2D->SetQuadVertexTexCoords( 3, XFactor, YFactor );
    
    // draw rectangle defined as a quad (4-vertex polygon)
    // (disable any transformations)
    OpenGL2D->SetTranslation( 0, 0 );
    OpenGL2D->ComposeTransform( false, false );
    OpenGL2D->DrawTexturedQuad();
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}
