// *****************************************************************************
    // start include guard
    #ifndef TEXTURE_HPP
    #define TEXTURE_HPP

    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings
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
        
        // reference point, relative to top-left
        int HotSpotX;
        int HotSpotY;
        
        // path of loaded image file
        std::string LoadedFile;
        
    public:
        
        // instance handling
        Texture();
       ~Texture();
        
        // resource handling
        void Load( const std::string& FileName );
        void Release();
        
        // drawing on screen
        void Draw( int HotSpotPositionX, int HotSpotPositionY ) const;
        void Draw( int RenderXMin, int RenderYMin, int RenderXMax, int RenderYMax ) const;
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
