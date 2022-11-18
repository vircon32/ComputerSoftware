// *****************************************************************************
    // include common Vircon headers
    #include "../VirconDefinitions/VirconDefinitions.hpp"
    #include "../VirconDefinitions/VirconDataStructures.hpp"
    
    // include project headers
    #include "Globals.hpp"
    #include "VirconEmulator.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// program state
bool GlobalLoopActive;

// video object
OpenGL2DContext OpenGL2D;

// instance of the Vircon virtual machine
VirconEmulator Vircon;
