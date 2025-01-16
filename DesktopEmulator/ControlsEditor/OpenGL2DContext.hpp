// *****************************************************************************
    // start include guard
    #ifndef OPENGL2DCONTEXT_HPP
    #define OPENGL2DCONTEXT_HPP
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


// =============================================================================
//      2D-SPECIALIZED OPENGL CONTEXT
// =============================================================================


class OpenGL2DContext
{
    public:
        
        // video context objects
        SDL_Window* Window;
        SDL_GLContext OpenGLContext;
        
        // graphical settings
        unsigned WindowWidth;
        unsigned WindowHeight;
        
    public:
        
        // instance handling
        OpenGL2DContext();
       ~OpenGL2DContext();
        
        // resource handling
        void CreateOpenGLWindow();
        void DestroyOpenGLWindow();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
