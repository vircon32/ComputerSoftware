// *****************************************************************************
    // include project headers
    #include "Definitions.hpp"
    #include "LogStream.hpp"
    #include "Texture.hpp"
    
    // include SDL2 headers
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
    #include <SDL2/SDL_image.h>     // [ SDL2 ] SDL_Image
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// seems like this is missing in the headers
#define GL_GENERATE_MIPMAP 0x8191


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
    
    // this will automatically generate mipmaps on texture modifications
    //glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
    
    // build the OpenGL texture from the SDL surface:
    GLenum Type = (LoadedImage->format->BytesPerPixel == 4)? GL_RGBA : GL_RGB;
    
    // (1) first we build an empty texture of the extented size
    glTexImage2D
    (
        GL_TEXTURE_2D,                          // texture is a 2D rectangle
        0,                                      // level of detail (0 = normal size)
        LoadedImage->format->BytesPerPixel,     // color components
        TextureWidth,                           // texture width in pixels
        TextureHeight,                          // texture height in pixels
        0,                                      // border width (must be 0 or 1)
        Type,                                   // buffer format for color components
        GL_UNSIGNED_BYTE,                       // each color component is a byte
        nullptr                                 // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create an empty OpenGL texture" );
    
    // (2) then we modify the part of our image
    glTexSubImage2D
    (
        GL_TEXTURE_2D,                          // texture is a 2D rectangle
        0,                                      // level of detail (0 = normal size)
        0,                                      // x offset
        0,                                      // y offset
        ImageWidth,                             // image width in pixels
        ImageHeight,                            // image height in pixels
        Type,                                   // buffer format for color components
        GL_UNSIGNED_BYTE,                       // each color component is a byte
        LoadedImage->pixels                     // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not copy the loaded SDL image to the OpenGL texture" );
    
    // delete the original SDL surface loaded by SDL_Image
    delete LoadedImage;
    
    // we can now disable mipmap updates
    //glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
    
    // specify a linear filter for both the minification and magnification
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );         
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
    
    // for this game none of our textures will be smoothed
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    
    // configure texture edges to NOT wrap (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    
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
}


// =============================================================================
//      TEXTURE: DRAWING ON SCREEN (WHOLE TEXTURE)
// =============================================================================


void Texture::Draw( const Vector2D& Position ) const
{
    // check that there is a texture
    if( !TextureID )
      return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // calculate texture render size
    float RenderWidth  = PixelWidth  * ImageWidth;
    float RenderHeight = PixelHeight * ImageHeight;
    
    // precalculate limit coordinates
    float RenderXMin = Position.x - HotSpotX;
    float RenderYMin = Position.y - HotSpotY;
    
    float RenderXMax = RenderXMin + RenderWidth;
    float RenderYMax = RenderYMin + RenderHeight;    
    
    // call the coordinate-based function
    Draw( RenderXMin, RenderYMin, RenderXMax, RenderYMax );
}

// -----------------------------------------------------------------------------

void Texture::Draw( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const
{
    // check that there is a texture
    if( !TextureID )
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
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {   
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f(       0,       0 );  glVertex2i( RenderXMinInt, RenderYMinInt );
        glTexCoord2f( XFactor,       0 );  glVertex2i( RenderXMaxInt, RenderYMinInt );
        glTexCoord2f( XFactor, YFactor );  glVertex2i( RenderXMaxInt, RenderYMaxInt );
        glTexCoord2f(       0, YFactor );  glVertex2i( RenderXMinInt, RenderYMaxInt );
    }
    glEnd();    
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}


// =============================================================================
//      TEXTURE: DRAWING ON SCREEN (RECTANGLE PART)
// =============================================================================


void Texture::DrawPart( int XMin, int YMin, int XMax, int YMax, float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const
{
    // check that there is a texture
    if( !TextureID )
      return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // precalculate texture coordinates
    // (to avoid distortion, we will round them to integers)
    float TextureXMin = (float)XMin / TextureWidth;
    float TextureXMax = (float)XMax / TextureWidth;
    float TextureYMin = (float)YMin / TextureHeight;
    float TextureYMax = (float)YMax / TextureHeight;
    
    // to avoid distortion, round screen coordinates to integers
    int RenderXMinInt = round( RenderXMin );
    int RenderYMinInt = round( RenderYMin );
    int RenderXMaxInt = round( RenderXMax );
    int RenderYMaxInt = round( RenderYMax );
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureXMin, TextureYMin );  glVertex2i( RenderXMinInt, RenderYMinInt );
        glTexCoord2f( TextureXMax, TextureYMin );  glVertex2i( RenderXMaxInt, RenderYMinInt );
        glTexCoord2f( TextureXMax, TextureYMax );  glVertex2i( RenderXMaxInt, RenderYMaxInt );
        glTexCoord2f( TextureXMin, TextureYMax );  glVertex2i( RenderXMinInt, RenderYMaxInt );
    }
    glEnd();
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}

// -----------------------------------------------------------------------------

void Texture::DrawPart( int XMin, int YMin, int XMax, int YMax, const Vector2D& Position, float ZoomX, float ZoomY ) const
{
    // check that there is a texture
    if( !TextureID ) return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // precalculate part render size
    float ScaleX = ZoomX * PixelWidth;
    float ScaleY = ZoomY * PixelHeight;
    
    float PartRenderWidth  = ScaleX * (XMax - XMin);
    float PartRenderHeight = ScaleY * (YMax - YMin);
    
    // precalculate screen coordinates
    //  - to avoid distortion, we will round them to integers
    //  - BUT, we overlap them less than 1 pixel so that zoom effects leave less gaps
    int RenderXMin = round( Position.x - HotSpotX*ScaleX );
    int RenderYMin = round( Position.y - HotSpotY*ScaleY );
    
    int RenderXMax = RenderXMin + PartRenderWidth + 0.2;
    int RenderYMax = RenderYMin + PartRenderHeight + 0.2;
    
    // precalculate texture coordinates
    float TextureXMin = (float)XMin / TextureWidth;
    float TextureXMax = (float)XMax / TextureWidth;
    float TextureYMin = (float)YMin / TextureHeight;
    float TextureYMax = (float)YMax / TextureHeight;
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureXMin, TextureYMin );  glVertex2i( RenderXMin, RenderYMin );
        glTexCoord2f( TextureXMax, TextureYMin );  glVertex2i( RenderXMax, RenderYMin );
        glTexCoord2f( TextureXMax, TextureYMax );  glVertex2i( RenderXMax, RenderYMax );
        glTexCoord2f( TextureXMin, TextureYMax );  glVertex2i( RenderXMin, RenderYMax );
    }
    glEnd();
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}


// =============================================================================
//      DRAWING WRAPPED TEXTURES
// =============================================================================


void Texture::SetTextureWrap( bool WrapInX, bool WrapInY )
{
    // check that there is a texture
    if( !TextureID ) return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // configure texture edges to NOT wrap (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapInX? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapInY? GL_REPEAT : GL_CLAMP );
}

// -----------------------------------------------------------------------------

void Texture::DrawWrapped( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax, int PixelOffsetX, int PixelOffsetY )
{
    // check that there is a texture
    if( !TextureID ) return;
    
    // select current texture
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // to avoid distortion, round screen coordinates to integers
    int RenderXMinInt = round( RenderXMin );
    int RenderYMinInt = round( RenderYMin );
    int RenderXMaxInt = round( RenderXMax );
    int RenderYMaxInt = round( RenderYMax );
    
    // calculate texture coordinates
    PixelOffsetX %= ImageWidth;
    PixelOffsetY %= ImageHeight;
    
    float XFactorMin = (float)PixelOffsetX/TextureWidth;
    float XFactorMax = (float)(PixelOffsetX+RenderXMaxInt-RenderXMinInt)/TextureWidth;
    
    float YFactorMin = (float)PixelOffsetY/TextureHeight;
    float YFactorMax = (float)(PixelOffsetY+RenderYMaxInt-RenderYMinInt)/TextureHeight;
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {   
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( XFactorMin, YFactorMin );  glVertex2i( RenderXMinInt, RenderYMinInt );
        glTexCoord2f( XFactorMax, YFactorMin );  glVertex2i( RenderXMaxInt, RenderYMinInt );
        glTexCoord2f( XFactorMax, YFactorMax );  glVertex2i( RenderXMaxInt, RenderYMaxInt );
        glTexCoord2f( XFactorMin, YFactorMax );  glVertex2i( RenderXMinInt, RenderYMaxInt );
    }
    glEnd();    
    
    // deselect texture
    glBindTexture( GL_TEXTURE_2D, 0 );
}
