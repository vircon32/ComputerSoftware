// *****************************************************************************
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "Globals.hpp"
    #include "Settings.hpp"
    #include "AudioOutput.hpp"
    #include "VideoOutput.hpp"
    #include "Logger.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <climits>          // [ ANSI C ] Numeric limits
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      CLASS: EMULATOR CONTROL
// =============================================================================


EmulatorControl::EmulatorControl()
{
    Paused = false;
}

// -----------------------------------------------------------------------------

EmulatorControl::~EmulatorControl()
{
    Terminate();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Initialize()
{
    Audio.Initialize();
    
    Console.SetCallbackClearScreen( Function_ClearScreen );
    Console.SetCallbackDrawQuad( Function_DrawQuad );
    Console.SetCallbackSetMultiplyColor( Function_SetMultiplyColor );
    Console.SetCallbackSetBlendingMode( Function_SetBlendingMode );
    Console.SetCallbackSelectTexture( Function_SelectTexture );
    Console.SetCallbackLoadTexture( Function_LoadTexture );
    Console.SetCallbackUnloadCartridgeTextures( Function_UnloadCartridgeTextures );
}

// -----------------------------------------------------------------------------

void EmulatorControl::Terminate()
{
    Console.SetPower( false );
    Audio.Terminate();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Pause()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || Paused ) return;
    
    // take pause actions
    Paused = true;
    Audio.Pause();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Resume()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || !Paused ) return;
    
    // take resume actions
    Paused = false;
    Audio.Resume();
}

// -----------------------------------------------------------------------------

bool EmulatorControl::IsPaused()
{
    return Paused;
}

// -----------------------------------------------------------------------------

void EmulatorControl::SetPower( bool On )
{
    Video.RenderToFramebuffer();
    Console.SetPower( On );

    if( On ) Audio.Reset();
    else Audio.Pause();
}

// -----------------------------------------------------------------------------

bool EmulatorControl::IsPowerOn()
{
    return Console.IsPowerOn();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Reset()
{
    LOG( "EmulatorControl::Reset" );
    Paused = false;
    Video.RenderToFramebuffer();
    Console.Reset();
    Audio.Reset();
}

// -----------------------------------------------------------------------------

void EmulatorControl::RunNextFrame()
{
    Console.RunNextFrame();
    Audio.ChangeFrame();
    
    // after running, ensure that all GPU
    // commands run in the current frame are drawn
    glFlush();   
}
