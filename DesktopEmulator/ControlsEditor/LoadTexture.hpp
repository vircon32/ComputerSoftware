// *****************************************************************************
    // start include guard
    #ifndef LOADTEXTURE_HPP
    #define LOADTEXTURE_HPP
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings

    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


// =============================================================================
//      LOADING TEXTURES FROM IMAGE FILES
// =============================================================================


GLuint LoadTexture( const std::string& FileName );
void ReleaseTexture( GLuint& TextureID );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
