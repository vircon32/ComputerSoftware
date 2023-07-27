// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    #include <string>       // [ C++ STL ] Strings
    
    // forward declarations for all needed classes
    // (to avoid needing to include all headers here)
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
//      CALLBACK FUNCTIONS FOR CONSOLE LOGIC
// =============================================================================


namespace CallbackFunctions
{
    // video functions callable by the console
    void ClearScreen( V32::GPUColor ClearColor );
    void DrawQuad( V32::GPUQuad& DrawnQuad );
    void SetMultiplyColor( V32::GPUColor MultiplyColor );
    void SetBlendingMode( int NewBlendingMode );
    void SelectTexture( int GPUTextureID );
    void LoadTexture( int GPUTextureID, void* Pixels );
    void UnloadCartridgeTextures();
    
    // log functions callable by the console
    void LogLine( const std::string& Message );
    void ThrowException( const std::string& Message );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
