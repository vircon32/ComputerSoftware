// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    
    // include project headers
    #include "OpenGL2D.hpp"
    #include "LogStream.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS TO CHECK AND LOG ERRORS
// =============================================================================


void LogSDLResult( const string& EntryText )
{
    string Result = SDL_GetError();
    LOG( EntryText + ": " + Result );
    
    if( Result[0] )
      THROW( "An SDL error happened" );
}

// -----------------------------------------------------------------------------

void LogOpenGLResult( const string& EntryText )
{
    GLenum Result = glGetError();
    string ResultString = (const char * )gluErrorString( Result );
    
    LOG( EntryText + ": " + ResultString );
    
    if( Result != GL_NO_ERROR )
      THROW( "An OpenGL error happened" );
}


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
    FBDepthTextureID = 0;
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
//      HANDLING OF SDL WINDOW + OPENGL CONTEXT
// =============================================================================


void OpenGL2DContext::CreateOpenGLWindow()
{
    LOG_SCOPE( "Creating OpenGL window" );
    
    // Use OpenGL 3.0 for FBO support
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    
    // Use double buffering
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
       "Input test",
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
    // (alternatively, use gladLoadGLLoader( SDL_GL_GetProcAddress ), but this fails)
    LOG( "Initializing GLAD" );
    
    if( !gladLoadGL() )
      THROW( "There was an error initializing GLAD" );
    
    // before continuing, check that we got
    // the required OpenGL version or higher
    string OpenGLVersion = (const char *)glGetString(GL_VERSION);
    LOG( "Started OpenGL version " + OpenGLVersion );
    
    // returned OpenGL version has format "X.X.X etc"
    int MajorVersion = OpenGLVersion[0] - '0';
    
    if( MajorVersion < 3 )
      THROW( string("OpenGL version 3.0 is not supported. Current version is ") + OpenGLVersion );
    
    // use vsync
    LOG( "Activating VSync" );
    SDL_GL_SetSwapInterval( 1 );
    
    // configure projection and viewport
    // (CAREFUL: do not call RenderToScreen for this, as it includes
    // a framebuffer binding ans GLEW is not initialized yet)
    LOG( "Setting 2D projection" );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glViewport( 0, 0, WindowWidth, WindowHeight );
    glOrtho( 0, WindowWidth, WindowHeight, 0, -1, 1 );
    
    // any render clipping is no longer necessary
    glDisable( GL_SCISSOR_TEST );
    
    // enable textures
    glEnable( GL_TEXTURE_2D );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::CreateFramebuffer()
{
    LOG_SCOPE( "Creating Framebuffer" );
    
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
            GL_RGB8, // GL_RGBA8,
            FramebufferWidth,
            FramebufferHeight,
            0,
            GL_RGB, // GL_RGBA,
            GL_UNSIGNED_BYTE,
            0
        );
        LogOpenGLResult( "glTexImage2D" );

        // our texture should be drawn to screen scaled with nearest neighbour
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        LogOpenGLResult( "glTexParameteri" );
    }
    
    // PART 2: DEPTH BUFFER
    { LOG_SCOPE( "Creating a render buffer" );
    
        glGenRenderbuffers( 1, &FBDepthTextureID );
        LogOpenGLResult( "glGenRenderbuffers" );
    }
    
    { LOG_SCOPE( "Binding the texture to the render buffer" );
    
        glBindRenderbuffer( GL_RENDERBUFFER, FBDepthTextureID );
        LogOpenGLResult( "glBindRenderbuffer" );
        
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FramebufferWidth, FramebufferHeight );
        LogOpenGLResult( "glRenderbufferStorage" );
        
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FBDepthTextureID );
        LogOpenGLResult( "glFramebufferRenderbuffer" );
    }
    
    // PART 3: FRAME BUFFER
    // Set "renderedTexture" as our colour attachement #0
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

void OpenGL2DContext::Destroy()
{
    // destroy in reverse order
    if( OpenGLContext )
      SDL_GL_DeleteContext( OpenGLContext );
    
    if( Window )
      SDL_DestroyWindow( Window );
}


// =============================================================================
//      RENDER FUNCTIONS
// =============================================================================


void OpenGL2DContext::RenderToScreen()
{
    // select the actual screen as the render target
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    // map viewport's rectangle to the window's client area
    glViewport( 0, 0, WindowWidth, WindowHeight );
    
    // establish orthographic projection and map its coordinates to the viewport
    // (NOTE: this sets the usual top-left origin convention)
    glOrtho( 0, WindowWidth, WindowHeight, 0, -1, 1 );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::RenderToFramebuffer()
{
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferID );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glViewport( 0, 0, FramebufferWidth, FramebufferHeight );
    
    // we are already inverting vertically when drawing to the screen
    // so do not invert here too or we will undo the effect
    glOrtho( 0, FramebufferWidth, 0, FramebufferHeight, -1, 1 );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::DrawFramebufferOnScreen()
{
    glBindTexture( GL_TEXTURE_2D, FBColorTextureID );
    
    float XFactor = (float)Constants::ScreenWidth  / FramebufferWidth;
    float YFactor = (float)Constants::ScreenHeight / FramebufferHeight;
    
    int QuadVertices[ 3 * 6 ] =
    {
                       0,                 0, 0, 
        (int)WindowWidth,                 0, 0, 
        (int)WindowWidth, (int)WindowHeight, 0,
        (int)WindowWidth, (int)WindowHeight, 0, 
                       0, (int)WindowHeight, 0, 
                       0,                 0, 0
    };
    
    float QuadTexCoords[ 2 * 6 ] =
    {
              0,       0, 
        XFactor,       0, 
        XFactor, YFactor,
        XFactor, YFactor, 
              0, YFactor, 
              0,       0
    };
    
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_INT, 0, QuadVertices );
    
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer( 2, GL_FLOAT, 0, QuadTexCoords );

    glDrawArrays( GL_TRIANGLES, 0, 2*3 );
}


// =============================================================================
//      CONFIGURATION FUNCTIONS
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
