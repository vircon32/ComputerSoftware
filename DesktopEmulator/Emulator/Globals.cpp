// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconDataStructures.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "Globals.hpp"
    #include "VirconEmulator.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


// program state
bool GlobalLoopActive;
bool MouseIsOnWindow;
string EmulatorFolder;

// GUI settings
list< string > RecentCartridgePaths;
list< string > RecentMemoryCardPaths;
string LastCartridgeDirectory;
string LastMemoryCardDirectory;


// =============================================================================
//      VIDEO OBJECTS
// =============================================================================


OpenGL2DContext OpenGL2D;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// video objects
Texture NoSignalTexture;

// instance of the Vircon virtual machine
VirconEmulator Vircon;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables()
{
    LOG( "Initializing global variables" );
    
    // we can safely use EmulatorFolder, it will
    // be filled even before this function is
    // called so that logging is initialized
    LastCartridgeDirectory = EmulatorFolder;
    LastMemoryCardDirectory = EmulatorFolder;
}
