// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Texture.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    
    // forward declarations for all needed classes
    namespace V32{ class V32Emulator; }
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


// program state
extern bool GlobalLoopActive;
extern bool MouseIsOnWindow;
extern std::string EmulatorFolder;
extern std::string BiosFileName;

// GUI settings
extern std::list< std::string > RecentCartridgePaths;
extern std::list< std::string > RecentMemoryCardPaths;
extern std::string LastCartridgeDirectory;
extern std::string LastMemoryCardDirectory;


// =============================================================================
//      VIDEO OBJECTS
// =============================================================================


extern OpenGL2DContext OpenGL2D;
extern std::string VertexShader;
extern std::string FragmentShader;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// video objects
extern Texture NoSignalTexture;

// instance of the Vircon virtual machine
extern V32::V32Emulator Vircon;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
