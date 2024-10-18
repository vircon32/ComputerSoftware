// *****************************************************************************
    // start include guard
    #ifndef SAVESTATES_HPP
    #define SAVESTATES_HPP
    
    // include common Vircon headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include C/C++ headers
    #include <string>         // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      STRUCTURES TO HANDLE CONSOLE STATE
// =============================================================================


typedef struct
{
    // all CPU registers
    V32::V32Word Registers[ 16 ];
    V32::V32Word InternalRegisters[ 3 ];
    
    // control flags
    int32_t Halted;
    int32_t Waiting;
}
CPUState;

// -----------------------------------------------------------------------------

typedef struct
{
    // all exposed GPU registers that are not
    // affected by texture and region selection
    // (12 words in total)
    V32::V32Word Registers[ 12 ];
    
    // configuration for the BIOS texture
    // (note that this ties each savestate to a particular BIOS)
    V32::GPUTexture BiosTexture;
    
    // configuration for cartridge textures
    V32::GPUTexture CartridgeTextures[ V32::Constants::GPUMaximumCartridgeTextures ];
}
GPUState;

// -----------------------------------------------------------------------------

typedef struct
{
    // same as SPUSound but not including the sound samples
    int32_t Length;
    int32_t PlayWithLoop;
    int32_t LoopStart;
    int32_t LoopEnd;
}
SPUSoundState;

// -----------------------------------------------------------------------------

typedef struct
{
    // all exposed SPU registers that are not
    // affected by channel and sound selection
    // (4 words in total)
    V32::V32Word Registers[ 4 ];
    
    // all SPU channels
    V32::SPUChannel Channels[ V32::Constants::SPUSoundChannels ];
    
    // configuration for the BIOS sound
    // (note that this ties each savestate to a particular BIOS)
    SPUSoundState BiosSound;
    
    // configuration for cartridge sounds
    SPUSoundState CartridgeSounds[ V32::Constants::SPUMaximumCartridgeSounds ];
}
SPUState;

// -----------------------------------------------------------------------------

typedef struct
{
    // the single gamepad controller exposed register
    // that is not affected by gamepad selection
    int32_t SelectedGamepad;
    
    // state of all gamepads (both real-time and provided)
    V32::GamepadState GamepadStates[ 2 * V32::Constants::GamepadPorts ];
}
GamepadControllerState;

// -----------------------------------------------------------------------------

typedef struct
{
    // RAM contents
    V32::V32Word RAM[ V32::Constants::RAMSize ];
    
    // state for minor chips
    V32::V32Word TimerRegisters[ 4 ];
    int32_t RNGCurrentValue;
    
    // NOTE 1: Power is assumed to be on when saving a
    // state. On a core this should always be the case.
    
    // NOTE 2: Screen contents are persistent, so the
    // drawing buffer should also be part of the state.
    // However taking and redrawing screenshots would
    // add significant size and complexity. Nearly all
    // games redraw the whole screen every frame, so
    // we will skip saving the screen.
}
OtherConsoleState;

// -----------------------------------------------------------------------------

typedef struct
{
    char Title[ 64 ];
    uint32_t Version;
    uint32_t Revision;
    int32_t ProgramROMSize;
    int32_t NumberOfTextures;
    int32_t NumberOfSounds;
}
ROMInfo;

// -----------------------------------------------------------------------------

typedef struct
{
    // data for the game itself; this is just a
    // basic attempt at distinguish different
    // games to prevent loading an incompatible
    // savestate and messing things up
    ROMInfo Game;
    
    // data to identify the BIOS; this is much
    // less important than the game itself, but
    // if different BIOSes are used to save and
    // load it can produce graphic errors
    ROMInfo Bios;
    
    // data for all stateful console components;
    // make GPU last so that we can adjust size
    // using a texture array (the last field)
    // only as large as needed for each game
    OtherConsoleState Others;
    GamepadControllerState GamepadController;
    CPUState CPU;
    SPUState SPU;
    GPUState GPU;
}
ConsoleState;


// =============================================================================
//      SERIALIZATION FUNCTIONS
// =============================================================================


// load/save to a memory buffer
void SaveState( ConsoleState* State );
void LoadState( const ConsoleState* State );

// load/save to a file
void SaveState( const std::string& FileName );
void LoadState( const std::string& FileName );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
