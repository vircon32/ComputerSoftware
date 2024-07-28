// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "EmulatorControl.hpp"
    #include "Globals.hpp"
    #include "Settings.hpp"
    #include "AudioOutput.hpp"
    #include "VideoOutput.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <climits>          // [ ANSI C ] Numeric limits
    #include <time.h>           // [ ANSI C ] Date and time
    
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
    AutoCardHandling = true;
}

// -----------------------------------------------------------------------------

EmulatorControl::~EmulatorControl()
{
    Terminate();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Initialize()
{
    // prepare audio system
    Audio.Initialize();
    
    // set console's video callbacks
    V32::Callbacks::ClearScreen = CallbackFunctions::ClearScreen;
    V32::Callbacks::DrawQuad = CallbackFunctions::DrawQuad;
    V32::Callbacks::SetMultiplyColor = CallbackFunctions::SetMultiplyColor;
    V32::Callbacks::SetBlendingMode = CallbackFunctions::SetBlendingMode;
    V32::Callbacks::SelectTexture = CallbackFunctions::SelectTexture;
    V32::Callbacks::LoadTexture = CallbackFunctions::LoadTexture;
    V32::Callbacks::UnloadCartridgeTextures = CallbackFunctions::UnloadCartridgeTextures;
    V32::Callbacks::UnloadBiosTexture = CallbackFunctions::UnloadBiosTexture;
    
    // set console's log callbacks
    V32::Callbacks::LogLine = CallbackFunctions::LogLine;
    V32::Callbacks::ThrowException = CallbackFunctions::ThrowException;
    
    // obtain current time
    time_t CreationTime;
    time( &CreationTime );
    struct tm* CreationTimeInfo = localtime( &CreationTime );
    
    // set console date and time
    // (Careful! C gives year counting from 1900)
    Console.SetCurrentDate( CreationTimeInfo->tm_year + 1900, CreationTimeInfo->tm_yday );
    Console.SetCurrentTime( CreationTimeInfo->tm_hour, CreationTimeInfo->tm_min, CreationTimeInfo->tm_sec );
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

void EmulatorControl::SetCardHandling( bool Auto )
{
    AutoCardHandling = Auto;
}

// -----------------------------------------------------------------------------

bool EmulatorControl::IsCardHandlingAuto()
{
    return AutoCardHandling;
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
    
    // ensure that all queued quads are rendered
    Video.RenderQuadQueue();
    
    // after running, ensure that all GPU
    // commands run in the current frame are drawn
    glFlush();   
}
