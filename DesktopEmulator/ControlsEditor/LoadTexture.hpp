// *****************************************************************************
    // start include guard
    #ifndef LOADTEXTURE_HPP
    #define LOADTEXTURE_HPP

    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings
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
