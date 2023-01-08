// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    
    // include project headers
    #include "OpenGL2DContext.hpp"
    #include "LogStream.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS TO CHECK AND LOG ERRORS
// =============================================================================


void ClearOpenGLErrors()
{
    while( true )
      if( glGetError() == GL_NO_ERROR )
        return;
}

// -----------------------------------------------------------------------------

// these are not defined in GLAD v1
#define GL_STACK_OVERFLOW  0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_TABLE_TOO_LARGE 0x8031

// use this instead of gluErrorString: it is easy to
// implement, and allows us to not depend on GLU/GLUT
string GLErrorString( GLenum ErrorCode )
{
    switch( ErrorCode )
    {
        // opengl 2 errors (8)
        case GL_NO_ERROR:           return "GL_NO_ERROR";
        case GL_INVALID_ENUM:       return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:      return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:  return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:     return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:    return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:      return "GL_OUT_OF_MEMORY";
        case GL_TABLE_TOO_LARGE:    return "GL_TABLE_TOO_LARGE";
        
        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        
        default:
            return "Unknown error";
  }
}

// -----------------------------------------------------------------------------

void LogOpenGLResult( const string& EntryText )
{
    GLenum Result = glGetError();
    string ResultString = GLErrorString( Result );
    
    LOG( EntryText + ": " + ResultString );
    
    if( Result != GL_NO_ERROR )
      THROW( "An OpenGL error happened" );
}


// =============================================================================
//      GLSL CODE FOR SHADERS
// =============================================================================


const string VertexShaderCode =
    "#version 100                                                                               \n"
    "                                                                                           \n"
    "uniform mediump mat4 TransformMatrix;                                                      \n"
    "attribute vec2 Position;                                                                   \n"
    "attribute vec2 InputTextureCoordinate;                                                     \n"
    "varying highp vec2 TextureCoordinate;                                                      \n"
    "                                                                                           \n"
    "void main()                                                                                \n"
    "{                                                                                          \n"
    "    // (1) first, apply transformations in the Vircon32 screen space (640x360, Y inverted) \n"
    "    gl_Position = TransformMatrix * vec4( Position.x, Position.y, 0.0, 1.0 );              \n"
    "                                                                                           \n"
    "    // (2) now convert coordinates to the standard OpenGL screen space                     \n"
    "                                                                                           \n"
    "    // x is transformed from (0.0,640.0) to (-1.0,+1.0)                                    \n"
    "    gl_Position.x = (gl_Position.x / (640.0/2.0)) - 1.0;                                   \n"
    "                                                                                           \n"
    "    // y is transformed from (0.0,360.0) to (+1.0,-1.0), so it undoes its inversion        \n"
    "    gl_Position.y = 1.0 - (gl_Position.y / (360.0/2.0));                                   \n"
    "                                                                                           \n"
    "    // (3) texture coordinate is just provided as is to the fragment shader                \n"
    "    // (it is only needed here because fragment shaders cannot take inputs directly)       \n"
    "    TextureCoordinate = InputTextureCoordinate;                                            \n"
    "}                                                                                          \n";

const string FragmentShaderCode =
    "#version 100                                                                    \n"
    "                                                                                \n"
    "uniform mediump vec4 MultiplyColor;                                             \n"
    "uniform sampler2D TextureUnit;                                                  \n"
    "varying highp vec2 TextureCoordinate;                                           \n"
    "                                                                                \n"
    "void main()                                                                     \n"
    "{                                                                               \n"
    "    gl_FragColor = MultiplyColor * texture2D( TextureUnit, TextureCoordinate ); \n"
    "}                                                                               \n";


// =============================================================================
//      OPENGL 2D CONTEXT: INSTANCE HANDLING
// =============================================================================


OpenGL2DContext::OpenGL2DContext()
{
    // window defaults to 1X, windowed
    FullScreen = false;
    WindowedZoomFactor = 1;
    WindowWidth  = WindowedZoomFactor * Constants::ScreenWidth;
    WindowHeight = WindowedZoomFactor * Constants::ScreenHeight;
    
    // framebuffer is not created yet
    FramebufferID = 0;
    FBColorTextureID = 0;
    FramebufferWidth = 0;
    FramebufferHeight = 0;
    
    // SDL & OpenGL contexts not created yet
    Window = nullptr;
    OpenGLContext = nullptr;
}

// -----------------------------------------------------------------------------

OpenGL2DContext::~OpenGL2DContext()
{
    Destroy();
}


// =============================================================================
//      OPENGL 2D CONTEXT: HANDLING SDL WINDOW + OPENGL CONTEXT
// =============================================================================


void OpenGL2DContext::CreateOpenGLWindow()
{
    LOG_SCOPE( "Creating OpenGL window" );
    
    // load default dynamic OpenGL libraries
    SDL_GL_LoadLibrary( nullptr );
    
    // choose what OpenGL version to request (system dependent)
    #ifdef __arm__
      // on Raspberry/ARM systems request OpenGL ES 2.0 for FBO support
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    #else
      // for other systems use the regular OpenGL Core 3.0
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
      SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    #endif
    
    
    // request double buffering
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
    // Create window
    LOG( "Creating window" );
    
    Uint32 WindowFlags = 
    (
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_SHOWN
    );
    
    Window = SDL_CreateWindow
    (
       "OpenGL Window",
       SDL_WINDOWPOS_UNDEFINED,
       SDL_WINDOWPOS_UNDEFINED,
       WindowWidth,
       WindowHeight,
       WindowFlags
    );
    
    if( !Window )
      THROW( string("Window cannot be created: ") + SDL_GetError() );
    
    // create an OpenGL rendering context
    LOG( "Creating OpenGL context" );
    OpenGLContext = SDL_GL_CreateContext( Window );
    
    if( !OpenGLContext )
      THROW( string("OpenGL context cannot be created: ") + SDL_GetError() );
    else
      LOG( "OpenGL context created successfully" );
    
    SDL_GL_MakeCurrent( Window, OpenGLContext );
    
    // GLAD loader has to be called after OpenGL is initialized (i.e. window is created)
    // but, before we make any calls related to framebuffer objects
    // (alternatively, use gladLoadGL() instead)
    LOG( "Initializing GLAD" );
    
    if( !gladLoadGLLoader( (GLADloadproc)SDL_GL_GetProcAddress ) )
      THROW( "There was an error initializing GLAD" );
    
    // log the version name for the received OpenGL context
    string OpenGLVersionName = (const char *)glGetString(GL_VERSION);
    LOG( "Started OpenGL version " + OpenGLVersionName );
    
    // check that we were not given a GL version lower than required
    int MajorVersion = 1, MinorVersion = 0;
    SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion );
    SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &MinorVersion );
    string ReceivedVersion = to_string( MajorVersion ) + "." + to_string( MinorVersion );
    
    #ifdef __arm__
    if( MajorVersion < 2 )
      THROW( string("OpenGL ES version 2.0 is not supported. Current version is ") + ReceivedVersion );
    #else
    if( MajorVersion < 3 )
      THROW( string("OpenGL version 3.0 is not supported. Current version is ") + ReceivedVersion );
    #endif
    
    // show basic OpenGL information
    LOG( "OpenGL renderer: " << (char*)glGetString( GL_RENDERER ) );
    LOG( "GLSL version: " << (char*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    
    // use vsync
    LOG( "Activating VSync" );
    SDL_GL_SetSwapInterval( 1 );
    
    // configure viewport
    // (CAREFUL: do not call RenderToScreen for this, as it includes
    // a framebuffer binding and fraebuffer is not created yet)
    LOG( "Setting 2D viewport" );
    glViewport( 0, 0, WindowWidth, WindowHeight );
    
    // any render clipping is no longer necessary
    glDisable( GL_SCISSOR_TEST );
    
    // enable textures
    glEnable( GL_TEXTURE_2D );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::CreateFramebuffer()
{
    LOG_SCOPE( "Creating Framebuffer" );
    ClearOpenGLErrors();
    
    // create our frame buffer and select it
    {   LOG_SCOPE( "Creating Framebuffer object" );
    
        glGenFramebuffers( 1, &FramebufferID );
        LogOpenGLResult( "glGenFramebuffers" );
        
        glBindFramebuffer( GL_FRAMEBUFFER, FramebufferID );
        LogOpenGLResult( "glBindFramebuffer" );
    }
    
    // PART 1: COLOR BUFFER
    
    // choose the needed dimensions
    FramebufferWidth  = NextPowerOf2( Constants::ScreenWidth  );
    FramebufferHeight = NextPowerOf2( Constants::ScreenHeight );
    
    // create our texture for the frame buffer and select it
    { LOG_SCOPE( "Creating a new texture" );
        
        glGenTextures( 1, &FBColorTextureID );
        LogOpenGLResult( "glGenTextures" );
        
        glBindTexture( GL_TEXTURE_2D, FBColorTextureID );
        LogOpenGLResult( "glBindTexture" );
        
        // Give an empty image to OpenGL ( the last "0" )    
        glTexImage2D
        (
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            FramebufferWidth,
            FramebufferHeight,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            0
        );
        LogOpenGLResult( "glTexImage2D" );

        // our texture should be drawn to screen scaled with nearest neighbour
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        LogOpenGLResult( "glTexParameteri" );
    }
    
    // PART 2: FRAME BUFFER
    // Set our color texture as framebuffer's colour attachment #0
    { LOG_SCOPE( "Binding the render buffer to the Framebuffer" );
        
        if( glFramebufferTexture2D == nullptr )
          LOG( "Function glFramebufferTexture is not linked!" );
        
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBColorTextureID, 0 );
        LogOpenGLResult( "glFramebufferTexture" );
        
        LOG( "Checking status of the created Framebuffer" );
        GLenum FBOStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
        LogOpenGLResult( "glCheckFramebufferStatus" );
        
        if( FBOStatus != GL_FRAMEBUFFER_COMPLETE )
          THROW( string("Framebuffer status is not complete. Status: ") + (const char*)glGetString( FBOStatus ) );
        
        LOG( "Framebuffer status OK" );
        
        // Set the list of draw buffers.
        GLenum ColorBuffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers( 1, ColorBuffers );
        LogOpenGLResult( "glDrawBuffers" );
    }
}

// -----------------------------------------------------------------------------

bool OpenGL2DContext::CompileShaderProgram()
{
    GLuint VertexShaderID = 0;
    GLuint FragmentShaderID = 0;
    int Success;
    
    ClearOpenGLErrors();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 1: Compile our vertex shader
    VertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    const char *VertexShaderPointer = VertexShaderCode.c_str();
    glShaderSource( VertexShaderID, 1, &VertexShaderPointer, nullptr );
    glCompileShader( VertexShaderID );
    glGetShaderiv( VertexShaderID, GL_COMPILE_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( VertexShaderID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( VertexShaderID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( "ERROR: Vertex shader compilation failed: " << GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Vertex shader compiled successfully! ID = " << VertexShaderID );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 2: Compile our fragment shader
    FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );
    const char *FragmentShaderPointer = FragmentShaderCode.c_str();
    glShaderSource( FragmentShaderID, 1, &FragmentShaderPointer, nullptr );
    glCompileShader( FragmentShaderID );
    glGetShaderiv( FragmentShaderID, GL_COMPILE_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( FragmentShaderID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( FragmentShaderID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( "ERROR: Fragment shader compilation failed: " << GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Fragment shader compiled successfully! ID = " << FragmentShaderID );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PART 3: Link our compiled shaders to form a GLSL program
    ShaderProgramID = glCreateProgram();
    glAttachShader( ShaderProgramID, VertexShaderID );
    glAttachShader( ShaderProgramID, FragmentShaderID );
    glLinkProgram( ShaderProgramID );
    
    glGetProgramiv( ShaderProgramID, GL_LINK_STATUS, &Success );
    
    if( !Success )
    {
        GLint GLInfoLogLength;
        glGetShaderiv( ShaderProgramID, GL_INFO_LOG_LENGTH, &GLInfoLogLength );
        
        GLchar* GLInfoLog = new GLchar[ GLInfoLogLength + 1 ];
        glGetShaderInfoLog( ShaderProgramID, GLInfoLogLength, nullptr, GLInfoLog );    
        
        LOG( "ERROR: Linking shader program failed: " << GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        return false;
    }
    
    LOG( "Shader program linked successfully! ID = " << ShaderProgramID );
    
    // clean-up temporary compilation objects
    glDetachShader( ShaderProgramID, VertexShaderID );
    glDetachShader( ShaderProgramID, FragmentShaderID );
    glDeleteShader( VertexShaderID );
    glDeleteShader( FragmentShaderID );
    
    return true;
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::InitRendering()
{
    LOG_SCOPE( "Initializing rendering" );
    ClearOpenGLErrors();
    
    // compile our shader program
    LOG( "Compiling GLSL shader program" );
    
    if( !CompileShaderProgram() )
      THROW( "Cannot compile GLSL shader program" );
    
    // now we can enable our program
    glUseProgram( ShaderProgramID );
    
    // find the position for all our input variables within the shader program
    PositionsLocation = glGetAttribLocation( ShaderProgramID, "Position" );
    TexCoordsLocation = glGetAttribLocation( ShaderProgramID, "InputTextureCoordinate" );
    
    // find the position for all our input uniforms within the shader program
    TextureUnitLocation = glGetUniformLocation( ShaderProgramID, "TextureUnit" );
    MultiplyColorLocation = glGetUniformLocation( ShaderProgramID, "MultiplyColor" );
    TransformMatrixLocation = glGetUniformLocation( ShaderProgramID, "TransformMatrix" );
    
    // on a core OpenGL profile, we need this since
    // the default VAO is not valid!
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );
    
    // we will also need this for a core OpenGL
    // profile. For an OpenGL ES profile, instead,
    // it is enough to just use VAO without VBO
    glGenBuffers( 1, &VBOPositions );
    glGenBuffers( 1, &VBOTexCoords );
    
    // bind our textures to GPU's texture unit 0
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );      // set no texture until we load one
    glEnable( GL_TEXTURE_2D );
    
    // initialize our transform parameters to neutral
    TransformMatrix.LoadIdentity();
    SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    
    // create a white texture to draw solid color
    CreateWhiteTexture();
    
    // allocate memory for vertex positions in the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOPositions );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        8 * sizeof( GLint ),
        QuadPositionCoords,
        GL_DYNAMIC_DRAW
    );
    
    // define format for vertex positions
    glVertexAttribPointer
    (
        PositionsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (x,y)
        GL_INT,             // each component is of type GLint
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( PositionsLocation );
    
    // allocate memory for texture coordinates in the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOTexCoords );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        8 * sizeof( GLfloat ),
        QuadTextureCoords,
        GL_DYNAMIC_DRAW
    );
    
    // define format for texture coordinates
    glVertexAttribPointer
    (
        TexCoordsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (u,v)
        GL_FLOAT,           // each component is of type GLFloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( TexCoordsLocation );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::CreateWhiteTexture()
{
    LOG_SCOPE( "Creating white texture" );
    ClearOpenGLErrors();
    
    // create new texture ID
    glGenTextures( 1, &WhiteTextureID );
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // create our texture from 1 single white pixel
    uint8_t WhitePixel[ 4 ] = { 255, 255, 255, 255 };
    
    glTexImage2D
    (
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1, 1,       // width, height
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        WhitePixel
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create 1x1 white texture to draw solid colors" );
    
    // configure the texture
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::Destroy()
{
    // destroy in reverse order
    if( OpenGLContext )
      SDL_GL_DeleteContext( OpenGLContext );
    
    if( Window )
      SDL_DestroyWindow( Window );
}


// =============================================================================
//      OPENGL 2D CONTEXT: VIEW CONFIGURATION FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetWindowZoom( int ZoomFactor )
{
    // exit full screen
    SDL_SetWindowFullscreen( Window, 0 );
    FullScreen = false;
    
    // determine window size
    WindowedZoomFactor = ZoomFactor;
    WindowWidth  = Constants::ScreenWidth * WindowedZoomFactor;
    WindowHeight = Constants::ScreenHeight * WindowedZoomFactor;
    
    // resize and center SDL window
    SDL_SetWindowSize( Window, WindowWidth, WindowHeight );
    SDL_SetWindowPosition( Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetFullScreen()
{
    // set SDL window to full screen mode
    // (this flag means that we keep the desktop video mode)
    SDL_SetWindowFullscreen( Window, SDL_WINDOW_FULLSCREEN_DESKTOP );
    FullScreen = true;
    
    // update our window size variables to desktop size
    SDL_GetWindowSize( Window, (int*)(&WindowWidth), (int*)(&WindowHeight) );
}


// =============================================================================
//      OPENGL 2D CONTEXT: FRAMEBUFFER RENDER FUNCTIONS
// =============================================================================


void OpenGL2DContext::RenderToScreen()
{
    // select the actual screen as the render target
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // map viewport's rectangle to the window's client area
    glViewport( 0, 0, WindowWidth, WindowHeight );
    
    // reset transforms
    TransformMatrix.LoadIdentity();
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::RenderToFramebuffer()
{
    // select framebuffer as the render target
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferID );
    
    // map viewport's rectangle to the framebuffer's screen area
    glViewport( 0, 0, Constants::ScreenWidth, Constants::ScreenHeight );
    
    // reset transforms
    TransformMatrix.LoadIdentity();
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::DrawFramebufferOnScreen()
{
    // 2 framebuffers can be bound for reading and
    // writing separately; we can use this to copy
    glBindFramebuffer( GL_READ_FRAMEBUFFER, FramebufferID );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
    
    // now perform the copy
    glBlitFramebuffer
    (
        0, 0,                                               // source top-left
        Constants::ScreenWidth, Constants::ScreenHeight,    // source bottom-right
        0, 0,                                               // destination top-left
        (int)WindowWidth, (int)WindowHeight,                // destination bottom-right
        GL_COLOR_BUFFER_BIT,                                // which bitfield to copy
        GL_NEAREST                                          // interpolation mode
    );    
}


// =============================================================================
//      OPENGL 2D CONTEXT: COLOR FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetMultiplyColor( GPUColor NewMultiplyColor )
{
    MultiplyColor = NewMultiplyColor;
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetBlendingMode( IOPortValues BlendingMode )
{
    switch( BlendingMode )
    {
        case IOPortValues::GPUBlendingMode_Alpha:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glBlendEquation( GL_FUNC_ADD );
            break;
            
        case IOPortValues::GPUBlendingMode_Add:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            glBlendEquation( GL_FUNC_ADD );
            break;
            
        case IOPortValues::GPUBlendingMode_Subtract:
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
            break;
        
        default:
            // ignore invalid values
            break;
    }
}


// =============================================================================
//      OPENGL 2D CONTEXT: 2D TRANSFORM FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetTranslation( int TranslationX, int TranslationY )
{
    TranslationMatrix.LoadIdentity();
    TranslationMatrix.Components[ 0 ][ 3 ] = TranslationX;
    TranslationMatrix.Components[ 1 ][ 3 ] = TranslationY;    
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetScale( float ScaleX, float ScaleY )
{
    ScalingMatrix.LoadIdentity();
    ScalingMatrix.Components[ 0 ][ 0 ] = ScaleX;
    ScalingMatrix.Components[ 1 ][ 1 ] = ScaleY;
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetRotation( float AngleZ )
{
    RotationMatrix.LoadIdentity();
    RotationMatrix.Components[ 0 ][ 0 ] =  cos( AngleZ );
    RotationMatrix.Components[ 0 ][ 1 ] = -sin( AngleZ );
    RotationMatrix.Components[ 1 ][ 0 ] =  sin( AngleZ );
    RotationMatrix.Components[ 1 ][ 1 ] =  cos( AngleZ );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::ComposeTransform( bool ScalingEnabled, bool RotationEnabled )
{
    TransformMatrix = TranslationMatrix;
    if( RotationEnabled ) TransformMatrix *= RotationMatrix;
    if( ScalingEnabled  ) TransformMatrix *= ScalingMatrix;
}


// =============================================================================
//      OPENGL 2D CONTEXT: BASE RENDER FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetQuadVertexPosition( int Vertex, int x, int y )
{
    QuadPositionCoords[ 2*Vertex ] = x;
    QuadPositionCoords[ 2*Vertex + 1 ] = y;
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetQuadVertexTexCoords( int Vertex, float u, float v )
{
    QuadTextureCoords[ 2*Vertex ] = u;
    QuadTextureCoords[ 2*Vertex + 1 ] = v;
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::DrawTexturedQuad()
{
    // PART 1: Update uniforms (i.e. shader globals)
    // - - - - - - - - - - - - - - - - - - - - - - - -
    
    // tell the GPU which of its texture processors to use
    glUniform1i( TextureUnitLocation, 0 );  // texture unit 0 is for decal textures
    
    // send our multiply color to the GPU
    glUniform4f
    (
        MultiplyColorLocation,      // location (0-based index) within the shader program
        MultiplyColor.R / 255.0,    // the 4 color components (RGBA) in range [0.0-1.0]
        MultiplyColor.G / 255.0,
        MultiplyColor.B / 255.0,
        MultiplyColor.A / 255.0
    );
    
    // send our vertex transform matrix to the GPU
    glUniformMatrix4fv
    (
        TransformMatrixLocation,                // location (0-based index) within the shader program
        1,                                      // number of matrices (only 1)
        GL_TRUE,                                // transpose (our arrays represent rows, not columns)
        &TransformMatrix.Components[ 0 ][ 0 ]   // pointer to the data
    );
    
    // PART 2: Send attributes (i.e. shader input variables)
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    glBindBuffer( GL_ARRAY_BUFFER, VBOPositions );

    // send updated vertex positions to the GPU
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        8 * sizeof( GLint ),
        QuadPositionCoords
    );
    
    // define storage and format for vertex positions
    glVertexAttribPointer
    (
        PositionsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (x,y)
        GL_INT,             // each component is of type GLint
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( PositionsLocation );
    
    // send updated vertex texture coordinates to the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOTexCoords );
    
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        8 * sizeof( GLfloat ),
        QuadTextureCoords
    );
    
    // define storage and format for texture coordinates
    glVertexAttribPointer
    (
        TexCoordsLocation,  // location (0-based index) within the shader program
        2,                  // 2 components per vertex (u,v)
        GL_FLOAT,           // each component is of type GLFloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        nullptr             // pointer to the array
    );
    
    glEnableVertexAttribArray( TexCoordsLocation );
    
    // PART 3: Draw geometry
    // - - - - - - - - - - - - -
    
    // draw the quad as 2 connected triangles
    glDrawArrays
    (
        GL_TRIANGLE_STRIP,  // determines order and interpretation of succesive vertices
        0,                  // begin from the first index
        4                   // use 4 consecutive indices
    );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::ClearScreen( GPUColor ClearColor )
{
    // temporarily replace multiply color with clear color
    GPUColor PreviousMultiplyColor = MultiplyColor;
    MultiplyColor = ClearColor;
    
    // temporarily reset transformation
    Matrix4D PreviousTransformMatrix;
    TransformMatrix.LoadIdentity();
    
    // bind white texture
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // set a full-screen quad with the same texture pixel
    SetQuadVertexPosition( 0,                      0,                       0 );
    SetQuadVertexPosition( 1, Constants::ScreenWidth,                       0 );
    SetQuadVertexPosition( 2,                      0, Constants::ScreenHeight );
    SetQuadVertexPosition( 3, Constants::ScreenWidth, Constants::ScreenHeight );
    
    SetQuadVertexTexCoords( 0, 0.5, 0.5 );
    SetQuadVertexTexCoords( 1, 0.5, 0.5 );
    SetQuadVertexTexCoords( 2, 0.5, 0.5 );
    SetQuadVertexTexCoords( 3, 0.5, 0.5 );
    
    // draw quad as "textured"
    DrawTexturedQuad();
    
    // restore previous render settings
    MultiplyColor = PreviousMultiplyColor;
    TransformMatrix = PreviousTransformMatrix;
}
