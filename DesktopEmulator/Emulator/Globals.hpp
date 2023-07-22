// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <list>         // [ C++ STL ] Lists
    #include <string>       // [ C++ STL ] Strings
    
    // forward declarations for all needed classes
    namespace V32{ class V32Console; }
    class OpenGL2DContext;
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
extern OpenGL2DContext OpenGL2D;
extern AudioOutput Audio;

// video resources
extern Texture NoSignalTexture;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables();


// =============================================================================
//      EMULATOR-LEVEL CONTROL
// =============================================================================


extern bool Emulator_Paused;

// general use methods
void Emulator_Initialize();
void Emulator_Terminate();

void Emulator_Pause();
void Emulator_Resume();
bool Emulator_IsPaused();

void Emulator_SetPower( bool On );
bool Emulator_IsPowerOn();
void Emulator_Reset();
void Emulator_RunNextFrame();

//void Emulator_Update( float DeltaTime );
//void Emulator_ProcessEvent( SDL_Event Event );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
