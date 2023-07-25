// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Logger.hpp"
    
    // include project headers
    #include "OpenGL2DContext.hpp"
    
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
//      OPENGL 2D CONTEXT: INSTANCE HANDLING
// =============================================================================


OpenGL2DContext::OpenGL2DContext()
{
    // these are constant for this program
    WindowWidth  = 640;
    WindowHeight = 360;
    
    // SDL & OpenGL contexts not created yet
    Window = nullptr;
    OpenGLContext = nullptr;
}

// -----------------------------------------------------------------------------

OpenGL2DContext::~OpenGL2DContext()
{
    DestroyOpenGLWindow();
}


// =============================================================================
//      OPENGL 2D CONTEXT: HANDLING SDL WINDOW + OPENGL CONTEXT
// =============================================================================


void OpenGL2DContext::CreateOpenGLWindow()
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
    
    // enable textures
    glEnable( GL_TEXTURE_2D );
    
    LOG( "Finished creating OpenGL window" );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::DestroyOpenGLWindow()
{
    // destroy in reverse order
    if( OpenGLContext )
      SDL_GL_DeleteContext( OpenGLContext );
    
    if( Window )
      SDL_DestroyWindow( Window );
}
