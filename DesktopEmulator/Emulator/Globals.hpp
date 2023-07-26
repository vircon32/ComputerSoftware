// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include console logic headers
    #include "../ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    #include <string>       // [ C++ STL ] Strings
    
    // forward declarations for all needed classes
    namespace V32{ class V32Console; }
    class EmulatorControl;
    class GamepadsInput;
    class VideoOutput;
    class AudioOutput;
    class Texture;
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
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon virtual machine
extern V32::V32Console Console;

// wrappers for console I/O operation
extern EmulatorControl Emulator;
extern VideoOutput Video;
extern AudioOutput Audio;
extern GamepadsInput Gamepads;

// video resources
extern Texture NoSignalTexture;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables();


// =============================================================================
//      PLAIN FUNCTION INTERFACES
// =============================================================================


void Function_ClearScreen( V32::GPUColor ClearColor );
void Function_DrawQuad( V32::GPUQuad& DrawnQuad );
void Function_SetMultiplyColor( V32::GPUColor MultiplyColor );
void Function_SetBlendingMode( int NewBlendingMode );
void Function_SelectTexture( int GPUTextureID );
void Function_LoadTexture( int GPUTextureID, void* Pixels );
void Function_UnloadCartridgeTextures();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
