// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include project headers
    #include "OpenGL2DContext.hpp"
// *****************************************************************************


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// program state
extern bool GlobalLoopActive;

// video object
extern OpenGL2DContext OpenGL2D;

// instance of the Vircon virtual machine
class VirconEmulator;
extern VirconEmulator Vircon;


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
