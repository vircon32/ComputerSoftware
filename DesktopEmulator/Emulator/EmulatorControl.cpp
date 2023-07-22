// *****************************************************************************
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "Globals.hpp"
    #include "AudioOutput.hpp"
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
    Console.Reset();
    Audio.Reset();
}

// -----------------------------------------------------------------------------

void EmulatorControl::RunNextFrame()
{
    Console.RunNextFrame();
    Audio.ChangeFrame();
}

// -----------------------------------------------------------------------------

void EmulatorControl::ProcessEvent( SDL_Event Event )
{
    // pending
}
