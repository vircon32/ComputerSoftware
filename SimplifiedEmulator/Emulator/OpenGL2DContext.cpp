// *****************************************************************************
    // include common Vircon headers
    #include "../VirconDefinitions/VirconDefinitions.hpp"
    
    // include project headers
    #include "OpenGL2DContext.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      OPENGL 2D CONTEXT: INSTANCE HANDLING
// =============================================================================


OpenGL2DContext::OpenGL2DContext()
{
    // window defaults to 1X, windowed
    FullScreen = false;
    WindowWidth  = Constants::ScreenWidth;
    WindowHeight = Constants::ScreenHeight;
    
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
    cout << "Creating OpenGL window" << endl;
    
    // load default dynamic OpenGL libraries
    SDL_GL_LoadLibrary( nullptr );
    
    // request OpenGL 2.1, (no framebuffer support)
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    
    // request a legacy profile to use the fixed pipeline
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    
    // prevent double buffering so that
    // we can have screen persistence
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
    
    // Create window
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
      throw runtime_error( string("Window cannot be created: ") + SDL_GetError() );
    
    // create an OpenGL rendering context
    OpenGLContext = SDL_GL_CreateContext( Window );
    
    if( !OpenGLContext )
      throw runtime_error( string("OpenGL context cannot be created: ") + SDL_GetError() );
    
    SDL_GL_MakeCurrent( Window, OpenGLContext );
    
    // GLAD loader has to be called after OpenGL is initialized (i.e. window is created)
    // but, before we make any calls related to framebuffer objects
    // (alternatively, use gladLoadGL() instead)
    if( !gladLoadGLLoader( (GLADloadproc)SDL_GL_GetProcAddress ) )
      throw runtime_error( "There was an error initializing GLAD" );
    
    // log the version name for the received OpenGL context
    string OpenGLVersionName = (const char *)glGetString(GL_VERSION);
    cout << "Started OpenGL version " << OpenGLVersionName << endl;
    
    // check that we were not given a GL version lower than required
    int MajorVersion = 1, MinorVersion = 0;
    SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &MajorVersion );
    SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &MinorVersion );
    
    if( MajorVersion < 2 || (MajorVersion == 2 && MinorVersion < 1) )
      throw runtime_error( string("OpenGL version 2.1 is not supported. Current version is ") + OpenGLVersionName );
    
    // use vsync
    SDL_GL_SetSwapInterval( 1 );
    
    // configure viewport
    glViewport( 0, 0, WindowWidth, WindowHeight );
    
    // we are already inverting vertically when drawing to the
    // screen so invert here too and we will undo the effect
    glOrtho( 0, WindowWidth, WindowHeight, 0, -1, 1 );
    
    // any render clipping is no longer necessary
    glDisable( GL_SCISSOR_TEST );
    
    // enable textures
    glEnable( GL_TEXTURE_2D );
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


// =============================================================================
//      OPENGL 2D CONTEXT: VIEW CONFIGURATION FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetFullScreen()
{
    // set SDL window to full screen mode
    // (this flag means that we keep the desktop video mode)
    SDL_SetWindowFullscreen( Window, SDL_WINDOW_FULLSCREEN_DESKTOP );
    FullScreen = true;
    
    // update our window size variables to desktop size
    SDL_GetWindowSize( Window, (int*)(&WindowWidth), (int*)(&WindowHeight) );
    
    // make vieport cover the whole screen
    glViewport( 0, 0, WindowWidth, WindowHeight );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::ExitFullScreen()
{
    // exit full screen
    SDL_SetWindowFullscreen( Window, 0 );
    FullScreen = false;
    
    // determine window size
    WindowWidth  = Constants::ScreenWidth;
    WindowHeight = Constants::ScreenHeight;
    
    // make vieport cover the window
    glViewport( 0, 0, WindowWidth, WindowHeight );
    
    // resize and center SDL window
    SDL_SetWindowSize( Window, WindowWidth, WindowHeight );
    SDL_SetWindowPosition( Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
}


// =============================================================================
//      OPENGL 2D CONTEXT: COLOR FUNCTIONS
// =============================================================================


void OpenGL2DContext::SetClearColor( GPUColor ClearColor )
{
    glClearColor
    (
        ClearColor.R / 255.0,
        ClearColor.G / 255.0,
        ClearColor.B / 255.0,
        ClearColor.A / 255.0
    );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::SetMultiplyColor( GPUColor MultiplyColor )
{
    glColor4ub
    (
        MultiplyColor.R,
        MultiplyColor.G,
        MultiplyColor.B,
        MultiplyColor.A
    );
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
//      OPENGL 2D CONTEXT: RENDER FUNCTIONS
// =============================================================================


void OpenGL2DContext::ClearScreen()
{
    glClear( GL_COLOR_BUFFER_BIT );
}

// -----------------------------------------------------------------------------

void OpenGL2DContext::RenderFrame()
{
    SDL_GL_SwapWindow( Window );
}
