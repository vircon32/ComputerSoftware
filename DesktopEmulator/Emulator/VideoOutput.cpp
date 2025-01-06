// *****************************************************************************
    // include common Vircon headers
    #include "../VirconDefinitions/Constants.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "VideoOutput.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
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
    "attribute vec4 VertexInfo;                                                                 \n"
    "varying highp vec2 TextureCoordinate;                                                      \n"
    "                                                                                           \n"
    "void main()                                                                                \n"
    "{                                                                                          \n"
    "    // (1) first convert coordinates to the standard OpenGL screen space                   \n"
    "                                                                                           \n"
    "    // x is transformed from (0.0,640.0) to (-1.0,+1.0)                                    \n"
    "    gl_Position.x = (VertexInfo.x / (640.0/2.0)) - 1.0;                                    \n"
    "                                                                                           \n"
    "    // y is transformed from (0.0,360.0) to (+1.0,-1.0), so it undoes its inversion        \n"
    "    gl_Position.y = 1.0 - (VertexInfo.y / (360.0/2.0));                                    \n"
    "                                                                                           \n"
    "    // even in 2D we may also need to set z and w                                          \n"
    "    gl_Position.z = 0.0;                                                                   \n"
    "    gl_Position.w = 1.0;                                                                   \n"
    "                                                                                           \n"
    "    // (2) now texture coordinate is just provided as is to the fragment shader            \n"
    "    // (it is only needed here because fragment shaders cannot take inputs directly)       \n"
    "    TextureCoordinate = VertexInfo.zw;                                                     \n"
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
//      VIDEO OUTPUT: INSTANCE HANDLING
// =============================================================================


VideoOutput::VideoOutput()
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
    
    // default values
    SelectedTexture = -1;
    QueuedQuads = 0;
    
    // all texture IDs are initially 0
    BiosTextureID = 0;
    WhiteTextureID = 0;
    
    for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
      CartridgeTextureIDs[ i ] = 0;
    
    // initialize vertex indices; they are organized
    // assuming each quad will be given as 4 vertices,
    // as in a GL_TRIANGLE_STRIP
    for( int i = 0; i < QUAD_QUEUE_SIZE; i++ )
    {
        VertexIndices[ 6*i + 0 ] = 4*i + 0;
        VertexIndices[ 6*i + 1 ] = 4*i + 1;
        VertexIndices[ 6*i + 2 ] = 4*i + 2;
        VertexIndices[ 6*i + 3 ] = 4*i + 1;
        VertexIndices[ 6*i + 4 ] = 4*i + 2;
        VertexIndices[ 6*i + 5 ] = 4*i + 3;
    }
}

// -----------------------------------------------------------------------------

VideoOutput::~VideoOutput()
{
    Destroy();
}


// =============================================================================
//      VIDEO OUTPUT: HANDLING SDL WINDOW + OPENGL CONTEXT
// =============================================================================


void VideoOutput::CreateOpenGLWindow()
{
    LOG( "Creating OpenGL window" );
    
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
    ClearOpenGLErrors();
    int MajorVersion = 1;
    SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion );
    
    #ifdef __arm__
    if( glGetError() != GL_NO_ERROR || MajorVersion < 2 )
      THROW( "This computer does not support the minimum required OpenGL version (OpenGL ES 2.0)" );
    #else
    if( glGetError() != GL_NO_ERROR || MajorVersion < 3 )
      THROW( "This computer does not support the minimum required OpenGL version (OpenGL 3.0)" );
    #endif
    
    // show basic OpenGL information
    LOG( string("OpenGL renderer: ") + (char*)glGetString( GL_RENDERER ) );
    LOG( string("GLSL version: ") + (char*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    
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
    
    LOG( "Finished creating OpenGL window" );
}

// -----------------------------------------------------------------------------

void VideoOutput::CreateFramebuffer()
{
    LOG( "Creating Framebuffer" );
    ClearOpenGLErrors();
    
    // create our frame buffer and select it
    LOG( "Creating Framebuffer object" );
    
    glGenFramebuffers( 1, &FramebufferID );
    LogOpenGLResult( "glGenFramebuffers" );
    
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferID );
    LogOpenGLResult( "glBindFramebuffer" );
    
    // PART 1: COLOR BUFFER
    
    // choose the needed dimensions
    FramebufferWidth  = NextPowerOf2( Constants::ScreenWidth  );
    FramebufferHeight = NextPowerOf2( Constants::ScreenHeight );
    
    // create our texture for the frame buffer and select it
    LOG( "Creating a new texture" );
        
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
    
    // PART 2: FRAME BUFFER
    // Set our color texture as framebuffer's colour attachment #0
    LOG( "Binding the render buffer to the Framebuffer" );
    
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
    
    LOG( "Finished creating Framebuffer" );
}

// -----------------------------------------------------------------------------

bool VideoOutput::CompileShaderProgram()
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
        
        LOG( string("ERROR: Vertex shader compilation failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Vertex shader compiled successfully! ID = " + to_string( VertexShaderID ) );
    
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
        
        LOG( string("ERROR: Fragment shader compilation failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        VertexShaderID = 0;
        return false;
    }
    
    LOG( "Fragment shader compiled successfully! ID = " + to_string( FragmentShaderID ) );
    
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
        
        LOG( string("ERROR: Linking shader program failed: ") + (char*)GLInfoLog );
        delete GLInfoLog;
        
        glDeleteShader( VertexShaderID );
        glDeleteShader( FragmentShaderID );
        return false;
    }
    
    LOG( "Shader program linked successfully! ID = " + to_string( ShaderProgramID ) );
    
    // clean-up temporary compilation objects
    glDetachShader( ShaderProgramID, VertexShaderID );
    glDetachShader( ShaderProgramID, FragmentShaderID );
    glDeleteShader( VertexShaderID );
    glDeleteShader( FragmentShaderID );
    
    return true;
}

// -----------------------------------------------------------------------------

void VideoOutput::InitRendering()
{
    LOG( "Initializing rendering" );
    ClearOpenGLErrors();
    
    // compile our shader program
    LOG( "Compiling GLSL shader program" );
    
    if( !CompileShaderProgram() )
      THROW( "Cannot compile GLSL shader program" );
    
    // now we can enable our program
    glUseProgram( ShaderProgramID );
    
    // find the position for all our input variables within the shader program
    VertexInfoLocation = glGetAttribLocation( ShaderProgramID, "VertexInfo" );
    
    // find the position for all our input uniforms within the shader program
    TextureUnitLocation = glGetUniformLocation( ShaderProgramID, "TextureUnit" );
    MultiplyColorLocation = glGetUniformLocation( ShaderProgramID, "MultiplyColor" );
    
    // on a core OpenGL profile, we need this since
    // the default VAO is not valid!
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );
    
    // we will also need this for a core OpenGL
    // profile. For an OpenGL ES profile, instead,
    // it is enough to just use VAO without VBO
    glGenBuffers( 1, &VBOVertexInfo );
    glGenBuffers( 1, &VBOIndices );
    
    // bind our textures to GPU's texture unit 0
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );      // set no texture until we load one
    
    // initialize our multiply color to neutral
    SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    
    // initialize blending
    glEnable( GL_BLEND );
    SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // create a white texture to draw solid color
    CreateWhiteTexture();
    
    // allocate memory for vertex info in the GPU
    glBindBuffer( GL_ARRAY_BUFFER, VBOVertexInfo );
    
    glBufferData
    (
        GL_ARRAY_BUFFER,
        sizeof( QuadVerticesInfo ),
        QuadVerticesInfo,
        GL_STREAM_DRAW
    );
    
    // define format for vertex info
    glVertexAttribPointer
    (
        VertexInfoLocation, // location (0-based index) within the shader program
        4,                  // 4 components per vertex (x,y,tex_x,tex_y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        (void*)0            // starts at offset 0
    );
    
    glEnableVertexAttribArray( VertexInfoLocation );
    
    // allocate memory for vertex indices in the GPU
    // (vertices are given as triangle strip pairs)
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VBOIndices );
    
    glBufferData
    (
        GL_ELEMENT_ARRAY_BUFFER,
        QUAD_QUEUE_SIZE * 6 * sizeof( GLuint ),
        VertexIndices,
        GL_STATIC_DRAW
    );
    
    LOG( "Finished initializing rendering" );
}

// -----------------------------------------------------------------------------

void VideoOutput::CreateWhiteTexture()
{
    LOG( "Creating white texture" );
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
    
    LOG( "Finished creating white texture" );
}

// -----------------------------------------------------------------------------

void VideoOutput::Destroy()
{
    // release all textures
    if( OpenGLContext )
    {
        UnloadTexture( BiosTextureID );
        UnloadTexture( WhiteTextureID );
        
        for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
          if( CartridgeTextureIDs[ i ] != 0 )
            UnloadTexture( CartridgeTextureIDs[ i ] );
    }
    
    // destroy in reverse order
    if( OpenGLContext )
      SDL_GL_DeleteContext( OpenGLContext );
    
    if( Window )
      SDL_DestroyWindow( Window );
}


// =============================================================================
//      VIDEO OUTPUT: EXTERNAL CONTEXT ACCESS
// =============================================================================


SDL_Window* VideoOutput::GetWindow()
{
    return Window;
}

// -----------------------------------------------------------------------------

SDL_GLContext VideoOutput::GetOpenGLContext()
{
    return OpenGLContext;
}

// -----------------------------------------------------------------------------

GLuint VideoOutput::GetFramebufferID()
{
    return FramebufferID;
}


// =============================================================================
//      VIDEO OUTPUT: VIEW CONFIGURATION FUNCTIONS
// =============================================================================


void VideoOutput::SetWindowZoom( int ZoomFactor )
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

int VideoOutput::GetWindowZoom()
{
    return WindowedZoomFactor;
}

// -----------------------------------------------------------------------------

void VideoOutput::SetFullScreen()
{
    // set SDL window to full screen mode
    // (this flag means that we keep the desktop video mode)
    SDL_SetWindowFullscreen( Window, SDL_WINDOW_FULLSCREEN_DESKTOP );
    FullScreen = true;
    
    // update our window size variables to desktop size
    SDL_GetWindowSize( Window, (int*)(&WindowWidth), (int*)(&WindowHeight) );
}

// -----------------------------------------------------------------------------

bool VideoOutput::IsFullScreen()
{
    return FullScreen;
}

// -----------------------------------------------------------------------------

float VideoOutput::GetRelativeWindowWidth()
{
    return WindowWidth / Constants::ScreenWidth;
}


// =============================================================================
//      VIDEO OUTPUT: FRAMEBUFFER RENDER FUNCTIONS
// =============================================================================


void VideoOutput::RenderToScreen()
{
    // select the actual screen as the render target
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // map viewport's rectangle to the window's client area
    glViewport( 0, 0, WindowWidth, WindowHeight );
}

// -----------------------------------------------------------------------------

void VideoOutput::RenderToFramebuffer()
{
    // select framebuffer as the render target
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferID );
    
    // map viewport's rectangle to the framebuffer's screen area
    glViewport( 0, 0, Constants::ScreenWidth, Constants::ScreenHeight );
}

// -----------------------------------------------------------------------------

void VideoOutput::DrawFramebufferOnScreen()
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

// -----------------------------------------------------------------------------

void VideoOutput::BeginFrame()
{
    glEnable( GL_BLEND );
    SelectTexture( SelectedTexture );
    SetBlendingMode( BlendingMode );
    SetMultiplyColor( MultiplyColor );
    
    // tell the GPU which of its texture processors to use
    glUniform1i( TextureUnitLocation, 0 );  // texture unit 0 is for decal textures
    
    // define storage and format for vertex info
    glVertexAttribPointer
    (
        VertexInfoLocation, // location (0-based index) within the shader program
        4,                  // 4 components per vertex (x,y,tex_x,tex_y)
        GL_FLOAT,           // each component is of type GLfloat
        GL_FALSE,           // do not normalize values (convert directly to fixed-point)
        0,                  // no gap between values (adjacent in memory)
        (void*)0            // starts at offset 0
    );
    
    glEnableVertexAttribArray( VertexInfoLocation );
}


// =============================================================================
//      VIDEO OUTPUT: COLOR FUNCTIONS
// =============================================================================


void VideoOutput::SetMultiplyColor( GPUColor NewMultiplyColor )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
    MultiplyColor = NewMultiplyColor;
    
    // send our multiply color to the GPU
    glUniform4f
    (
        MultiplyColorLocation,      // location (0-based index) within the shader program
        MultiplyColor.R / 255.0,    // the 4 color components (RGBA) in range [0.0-1.0]
        MultiplyColor.G / 255.0,
        MultiplyColor.B / 255.0,
        MultiplyColor.A / 255.0
    );
}

// -----------------------------------------------------------------------------

GPUColor VideoOutput::GetMultiplyColor()
{
    return MultiplyColor;
}

// -----------------------------------------------------------------------------

void VideoOutput::SetBlendingMode( IOPortValues NewBlendingMode )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
    switch( NewBlendingMode )
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
            return;
    }
    
    BlendingMode = NewBlendingMode;
}

// -----------------------------------------------------------------------------

IOPortValues VideoOutput::GetBlendingMode()
{
    return BlendingMode;
}


// =============================================================================
//      VIDEO OUTPUT: BASE RENDER FUNCTIONS
// =============================================================================


void VideoOutput::AddQuadToQueue( const GPUQuad& Quad )
{
    // copy information from the received GPU quad
    const int SizePerQuad = 16 * sizeof( float );
    memcpy( &QuadVerticesInfo[ QueuedQuads * 16 ], &Quad.Vertices, SizePerQuad );
    
    // update the queue
    QueuedQuads++;
    
    // force queue draw if it becomes full
    if( QueuedQuads >= QUAD_QUEUE_SIZE )
      RenderQuadQueue();
}

// -----------------------------------------------------------------------------

void VideoOutput::RenderQuadQueue()
{
    if( QueuedQuads == 0 ) return;
    
    // send attributes (i.e. shader input variables)
    glBindBuffer( GL_ARRAY_BUFFER, VBOVertexInfo );

    // send updated vertex info to the GPU; note that
    // we would normally not update the whole buffer
    // every time, but some mobile GPUs have a bug
    // which causes very low performance on partial
    // GPU buffer updates
    glBufferSubData
    (
        GL_ARRAY_BUFFER,
        0,
        sizeof( QuadVerticesInfo ),
        QuadVerticesInfo
    );
    
    // draw the quad as 2 triangles
    glDrawElements
    (
        GL_TRIANGLES,         // independent triangles
        QueuedQuads * 6,      // number of indices
        GL_UNSIGNED_SHORT,    // format of indices
        (void*)0              // starts at offset 0
    );
    
    // reset the queue
    QueuedQuads = 0;
}

// -----------------------------------------------------------------------------

void VideoOutput::ClearScreen( GPUColor ClearColor )
{
    // temporarily replace multiply color with clear color
    GPUColor PreviousMultiplyColor = MultiplyColor;
    SetMultiplyColor( ClearColor );
    
    // bind white texture
    glBindTexture( GL_TEXTURE_2D, WhiteTextureID );
    
    // set a full-screen quad with the same texture pixel
    const GPUQuad ScreenQuad =
    {
        {
            // 4x (vertex position + texture coordinates)
            { 0, 0, 0.5, 0.5 },
            { Constants::ScreenWidth, 0, 0.5, 0.5 },
            { 0, Constants::ScreenHeight, 0.5, 0.5 },
            { Constants::ScreenWidth, Constants::ScreenHeight, 0.5, 0.5 }
        }
    };
    
    // draw this quad separately, since we are
    // using different render configurations
    AddQuadToQueue( ScreenQuad );
    RenderQuadQueue();
    
    // restore previous multiply color and texture
    SetMultiplyColor( PreviousMultiplyColor );
    SelectTexture( SelectedTexture );
}


// =============================================================================
//      VIDEO OUTPUT: TEXTURE HANDLING
// =============================================================================


void VideoOutput::LoadTexture( int GPUTextureID, void* Pixels )
{
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    // create a new OpenGL texture and select it
    glGenTextures( 1, OpenGLTextureID );
    glBindTexture( GL_TEXTURE_2D, *OpenGLTextureID );
    
    // check correct texture ID
    if( !OpenGLTextureID )
      THROW( "OpenGL failed to generate a new texture" );
    
    // clear OpenGL errors
    glGetError();
    
    // create an OpenGL texture from the received pixel data
    glTexImage2D
    (
        GL_TEXTURE_2D,              // texture is a 2D rectangle
        0,                          // level of detail (0 = normal size)
        GL_RGBA,                    // color components in the texture
        Constants::GPUTextureSize,  // texture width in pixels
        Constants::GPUTextureSize,  // texture height in pixels
        0,                          // border width (must be 0 or 1)
        GL_RGBA,                    // color components in the source
        GL_UNSIGNED_BYTE,           // each color component is a byte
        Pixels                      // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create an OpenGL texture from pixel data" );
    
    // textures must be scaled using only nearest neighbour
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // out-of-texture coordinates must clamp, not wrap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

// -----------------------------------------------------------------------------

void VideoOutput::UnloadTexture( int GPUTextureID )
{
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    glDeleteTextures( 1, OpenGLTextureID );
    *OpenGLTextureID = 0;
}

// -----------------------------------------------------------------------------

void VideoOutput::SelectTexture( int GPUTextureID )
{
    // we must render any pending quads before
    // applying any new render configurations
    RenderQuadQueue();
    
    SelectedTexture = GPUTextureID;
    GLuint* OpenGLTextureID = &BiosTextureID;
    
    if( GPUTextureID >= 0 )
      OpenGLTextureID = &CartridgeTextureIDs[ GPUTextureID ];
    
    glBindTexture( GL_TEXTURE_2D, *OpenGLTextureID );
}

// -----------------------------------------------------------------------------

int32_t VideoOutput::GetSelectedTexture()
{
    return SelectedTexture;
}
