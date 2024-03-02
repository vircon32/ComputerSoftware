// *****************************************************************************
    // start include guard
    #ifndef VIDEOOUTPUT_HPP
    #define VIDEOOUTPUT_HPP
    
    // include common Vircon headers
    #include "../VirconDefinitions/DataStructures.hpp"
    #include "../VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
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


class VideoOutput
{
    private:
        
        // video context objects
        SDL_Window* Window;
        SDL_GLContext OpenGLContext;
        
        // graphical settings
        unsigned WindowWidth;
        unsigned WindowHeight;
        unsigned WindowedZoomFactor;
        bool FullScreen;
        
        // arrays to hold buffer info
        GLfloat QuadVerticesInfo[ 16 ];
        
        // current color modifiers
        V32::GPUColor MultiplyColor;
        V32::IOPortValues BlendingMode;
        
        // OpenGL IDs of loaded textures
        GLuint BiosTextureID;
        GLuint CartridgeTextureIDs[ V32::Constants::GPUMaximumCartridgeTextures ];
        int32_t SelectedTexture;
        
        // white texture used to draw solid colors
        GLuint WhiteTextureID;
        
        // framebuffer object data
        GLuint FramebufferID;
        GLuint FBColorTextureID;
        unsigned FramebufferWidth;
        unsigned FramebufferHeight;
        
        // additional GL objects
        GLuint VAO;
        GLuint VBOVertexInfo;
        GLuint VBOIndices;
        GLuint ShaderProgramID;
        
        // positions of shader parameters
        GLuint VertexInfoLocation;
        GLuint TextureUnitLocation;
        GLuint MultiplyColorLocation;
        
    public:
        
        // instance handling
        VideoOutput();
       ~VideoOutput();
        
        // init functions
        void CreateOpenGLWindow();
        void CreateFramebuffer();
        bool CompileShaderProgram();
        void CreateWhiteTexture();
        void InitRendering();
        
        // release functions
        void Destroy();
        
        // external context access
        SDL_Window* GetWindow();
        SDL_GLContext GetOpenGLContext();
        GLuint GetFramebufferID();
        
        // view configuration
        void SetWindowZoom( int ZoomFactor );
        int GetWindowZoom();
        void SetFullScreen();
        bool IsFullScreen();
        float GetRelativeWindowWidth();
        
        // framebuffer render functions
        void RenderToScreen();
        void RenderToFramebuffer();
        void DrawFramebufferOnScreen();
        void BeginFrame();
        
        // color control functions
        void SetMultiplyColor( V32::GPUColor NewMultiplyColor );
        V32::GPUColor GetMultiplyColor();
        void SetBlendingMode( V32::IOPortValues BlendingMode );
        V32::IOPortValues GetBlendingMode();
        
        // render functions
        void DrawTexturedQuad( const V32::GPUQuad& Quad );
        void ClearScreen( V32::GPUColor ClearColor );
        
        // texture handling
        void SelectTexture( int GPUTextureID );
        void LoadTexture( int GPUTextureID, void* Pixels );
        void UnloadTexture( int GPUTextureID );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
