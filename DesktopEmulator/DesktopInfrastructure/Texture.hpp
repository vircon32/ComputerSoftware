// *****************************************************************************
    // start include guard
    #ifndef TEXTURE_HPP
    #define TEXTURE_HPP

    // include project headers
    #include "Definitions.hpp"
    #include "OpenGL2DContext.hpp"
    #include "Vector2D.hpp"
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


// =============================================================================
//      CLASS TO LOAD AN IMAGE FILE AS AN OPENGL TEXTURE
// =============================================================================


class Texture
{
    public:
        
        // owner GL context (needed to draw)
        OpenGL2DContext* OpenGL2D;
        
        // OpenGL texture data
        GLuint TextureID;
        unsigned TextureWidth;
        unsigned TextureHeight;
        unsigned ImageWidth;
        unsigned ImageHeight;
        std::string LoadedFile;
        
    public:
        
        // nominal on-screen dimensions
        float PixelWidth;
        float PixelHeight;
        
        // positioning point
        int HotSpotX;
        int HotSpotY;
        
    public:
        
        // instance handling
        Texture();
       ~Texture();
        
        // resource handling
        void SetOpenGLContext( OpenGL2DContext& OpenGL2D_ );
        void Load( const std::string& FileName );
        void Release();
        
        // drawing on screen
        void Draw( const Vector2D& HotSpotPosition ) const;
        void Draw( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const;
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
