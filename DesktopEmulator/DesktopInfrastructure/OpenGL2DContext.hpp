// *****************************************************************************
    // start include guard
    #ifndef OPENGL2DCONTEXT_HPP
    #define OPENGL2DCONTEXT_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDataStructures.hpp"
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include project headers
    #include "Definitions.hpp"
    #include "Matrix4D.hpp"
    
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
        unsigned WindowedZoomFactor;
        bool FullScreen;
        
        // video context objects
        SDL_Window* Window;
        SDL_GLContext OpenGLContext;
        
        // framebuffer object
        GLuint FramebufferID;
        GLuint FBColorTextureID;
        unsigned FramebufferWidth;
        unsigned FramebufferHeight;

        // additional GL objects
        GLuint VAO;
        GLuint VBOPositions;
        GLuint VBOTexCoords;
        GLuint ShaderProgramID;
        
        // positions of shader parameters
        GLuint PositionsLocation;
        GLuint TexCoordsLocation;
        GLuint TextureUnitLocation;
        GLuint MultiplyColorLocation;
        GLuint TransformMatrixLocation;
        
        // arrays to hold buffer info
        GLint QuadPositionCoords[ 8 ];
        GLfloat QuadTextureCoords[ 8 ];
        
        // transformation matrices
        Matrix4D TranslationMatrix, ScalingMatrix, RotationMatrix;
        Matrix4D TransformMatrix;
        
        // multiply color
        GPUColor MultiplyColor;
        
        // white texture used to draw solid colors
        GLuint WhiteTextureID;
        
    public:
        
        // instance handling
        OpenGL2DContext();
       ~OpenGL2DContext();
        
        // init functions
        void CreateOpenGLWindow();
        void CreateFramebuffer();
        bool CompileShaderProgram();
        void CreateWhiteTexture();
        void InitRendering();
        
        // release functions
        void Destroy();
        
        // view configuration
        void SetWindowZoom( int ZoomFactor );
        void SetFullScreen();
        
        // framebuffer render functions
        void RenderToScreen();
        void RenderToFramebuffer();
        void DrawFramebufferOnScreen();
        
        // color functions
        void SetMultiplyColor( GPUColor NewMultiplyColor );
        void SetBlendingMode( IOPortValues BlendingMode );
        
        // 2D transform functions
        void SetTranslation( int TranslationX, int TranslationY );
        void SetScale( float ScaleX, float ScaleY );
        void SetRotation( float AngleZ );
        void ComposeTransform( bool ScalingEnabled, bool RotationEnabled );
        
        // render functions
        void SetQuadVertexPosition( int Vertex, int x, int y );
        void SetQuadVertexTexCoords( int Vertex, float u, float v );
        void DrawTexturedQuad();
        void ClearScreen( GPUColor ClearColor );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
