// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/Texture.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/Logger.hpp"
    
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "AudioOutput.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


// program state
bool GlobalLoopActive;
bool MouseIsOnWindow;
string EmulatorFolder;
string BiosFileName;

// GUI settings
list< string > RecentCartridgePaths;
list< string > RecentMemoryCardPaths;
string LastCartridgeDirectory;
string LastMemoryCardDirectory;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon virtual machine
V32Console Console;

// wrappers for console I/O operation
OpenGL2DContext OpenGL2D;
AudioOutput Audio;

// video resources
Texture NoSignalTexture;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables()
{
    LOG( "Initializing global variables" );
    
    // default bios to load is standard bios
    BiosFileName = "StandardBios.v32";
    
    // we can safely use EmulatorFolder, it will
    // be filled even before this function is
    // called so that logging is initialized
    LastCartridgeDirectory = EmulatorFolder;
    LastMemoryCardDirectory = EmulatorFolder;
}


// =============================================================================
//      EMULATOR-LEVEL CONTROL
// =============================================================================


bool Emulator_Paused = false;

// -----------------------------------------------------------------------------

void Emulator_Initialize()
{
    Audio.Initialize();
}

// -----------------------------------------------------------------------------

void Emulator_Terminate()
{
    Console.SetPower( false );
    Audio.Terminate();
}

// -----------------------------------------------------------------------------

void Emulator_Pause()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || Emulator_Paused ) return;
    
    // take pause actions
    Emulator_Paused = true;
    Audio.Pause();
}

// -----------------------------------------------------------------------------

void Emulator_Resume()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || !Emulator_Paused ) return;
    
    // take resume actions
    Emulator_Paused = false;
    Audio.Resume();
}

// -----------------------------------------------------------------------------

bool Emulator_IsPaused()
{
    return Emulator_Paused;
}

// -----------------------------------------------------------------------------

void Emulator_SetPower( bool On )
{
    Console.SetPower( On );
    
    if( On ) Audio.Reset();
    else Audio.Pause();
}

// -----------------------------------------------------------------------------

bool Emulator_IsPowerOn()
{
    return Console.IsPowerOn();
}

// -----------------------------------------------------------------------------

void Emulator_Reset()
{
    LOG("Emulator_Reset");
    Emulator_Paused = false;
    Console.Reset();
    Audio.Reset();
}

// -----------------------------------------------------------------------------

void Emulator_RunNextFrame()
{
    Console.RunNextFrame();
    Audio.ChangeFrame();
}
