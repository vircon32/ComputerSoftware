// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/Texture.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/Logger.hpp"
    
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
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
EmulatorControl Emulator;
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
