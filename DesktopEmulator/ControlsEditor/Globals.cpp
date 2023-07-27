// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include controls editor headers
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


bool GlobalLoopActive = true;
string ProgramFolder;


// =============================================================================
//      VIDEO OBJECTS
// =============================================================================


OpenGL2DContext OpenGL2D;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// resources
char ProfileName[ 41 ];
unsigned GamepadTextureID;
