// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include controls editor headers
    #include "LoadTexture.hpp"
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      LOADING TEXTURES FROM IMAGE FILES
// =============================================================================


GLuint LoadTexture( const string& FileName )
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
    int ImageWidth  = LoadedImage->w;
    int ImageHeight = LoadedImage->h;
    
    // read image dimensions
    int TextureWidth  = NextPowerOf2( ImageWidth  );
    int TextureHeight = NextPowerOf2( ImageHeight );
    
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
    GLuint TextureID = 0;
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
    delete LoadedImage;
    
    // for this console none of our textures will be smoothed
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // configure texture edges to NOT wrap (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    return TextureID;
}

// -----------------------------------------------------------------------------

void ReleaseTexture( GLuint& TextureID )
{
    if( !TextureID ) return;
    
    // delete the OpenGL texture
    LOG( "Release texture" );
    glDeleteTextures( 1, &TextureID );
    TextureID = 0;
}
