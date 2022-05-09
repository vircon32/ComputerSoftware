// *****************************************************************************
    // start include guard
    #ifndef OPENGL2D_HPP
    #define OPENGL2D_HPP
    
    // include project headers
    #include "Definitions.hpp"
    
    // include SDL2 headers
    #include <SDL2/SDL.h>       // [ SDL2 ] Main header
    
    // this is needed by OpenGL headers under Windows
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #include <windef.h>
    #endif
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
    #if defined(__APPLE__)
      #include <OpenGL/glu.h>   // [ OpenGL ] Utility library
    #else
      #include <GL/glu.h>       // [ OpenGL ] Utility library
    #endif
// *****************************************************************************


// =============================================================================
//      2D-SPECIALIZED OPENGL CONTEXT
// =============================================================================


class OpenGL2DContext
{
    public:
        
        // graphical settings
        unsigned WindowWidth;
        unsigned WindowHeight;
        unsigned WindowedZoomFactor;
        bool FullScreen;
        
        // framebuffer
        GLuint FramebufferID;
        GLuint FBColorTextureID;
        GLuint FBDepthTextureID;
        unsigned FramebufferWidth;
        unsigned FramebufferHeight;

        // video objects
        SDL_Window* Window;
        SDL_GLContext OpenGLContext;
        
    public:
        
        // instance handling
        OpenGL2DContext();
       ~OpenGL2DContext();
        
        // init functions
        void CreateOpenGLWindow();
        void CreateFramebuffer();
        
        // release functions
        void Destroy();
        
        // render functions
        void RenderToScreen();
        void RenderToFramebuffer();
        void DrawFramebufferOnScreen();
        
        // configuration
        void SetWindowZoom( int ZoomFactor );
        void SetFullScreen();
};



// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
