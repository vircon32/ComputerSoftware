// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/Texture.hpp"
    
    // include C/C++ headers
    #include <map>      // [ C++ STL ] Maps
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


extern bool GlobalLoopActive;
extern std::string ProgramFolder;


// =============================================================================
//      VIDEO OBJECTS
// =============================================================================


extern OpenGL2DContext OpenGL2D;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


extern char ProfileName[ 41 ];
extern Texture GamepadTexture;


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
