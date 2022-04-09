// *****************************************************************************
    // start include guard
    #ifndef TEXTURE_HPP
    #define TEXTURE_HPP

    // include project headers
    #include "Definitions.hpp"
    #include "Vector2D.hpp"
    #include "Video.hpp"
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


// =============================================================================
//      CLASS TO LOAD AN IMAGE FILE AS AN OPENGL TEXTURE
// =============================================================================


class Texture
{
    public:
        
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
        void Load( const std::string& FileName );
        void Release();
        
        // drawing on screen (whole texture)
        void Draw( const Vector2D& Position ) const;
        void Draw( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const;
        
        // drawing on screen (rectangular part)
        void DrawPart( int XMin, int YMin, int XMax, int YMax, float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax ) const;
        void DrawPart( int XMin, int YMin, int XMax, int YMax, const Vector2D& Position, float ZoomX = 1, float ZoomY = 1) const;
        
        // we will use this functions just for the
        // case of wanting to wrap textures
        void SetTextureWrap( bool WrapInX, bool WrapInY );
        void DrawWrapped( float RenderXMin, float RenderYMin, float RenderXMax, float RenderYMax, int PixelOffsetX, int PixelOffsetY );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
