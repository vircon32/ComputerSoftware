// *****************************************************************************
    // start include guard
    #ifndef OPENGL2DCONTEXT_HPP
    #define OPENGL2DCONTEXT_HPP
    
    // include common Vircon headers
    #include "../VirconDefinitions/VirconDataStructures.hpp"
    #include "../VirconDefinitions/VirconEnumerations.hpp"
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>       // [ SDL2 ] Main header
    
    // this is needed by OpenGL headers under Windows
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #include <windef.h>
    #endif
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
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
        bool FullScreen;
        
        // video context objects
        SDL_Window* Window;
        SDL_GLContext OpenGLContext;
        
    public:
        
        // instance handling
        OpenGL2DContext();
       ~OpenGL2DContext();
        
        // context handling
        void CreateOpenGLWindow();
        void DestroyOpenGLWindow();
        
        // view configuration
        void SetFullScreen();
        void ExitFullScreen();
        
        // color treatment functions
        void SetClearColor( GPUColor ClearColor );
        void SetMultiplyColor( GPUColor MultiplyColor );
        void SetBlendingMode( IOPortValues BlendingMode );
        
        // render functions
        void ClearScreen();
        void RenderFrame();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
