// *****************************************************************************
    // start include guard
    #ifndef VIRCONEMULATOR_HPP
    #define VIRCONEMULATOR_HPP
    
    // include project headers
    #include "VirconCPU.hpp"
    #include "VirconGPU.hpp"
    #include "VirconSPU.hpp"
    #include "VirconTimer.hpp"
    #include "VirconRNG.hpp"
    #include "VirconMemory.hpp"
    #include "VirconGamepadController.hpp"
    #include "VirconCartridgeController.hpp"
    #include "VirconMemoryCardController.hpp"
    #include "VirconNullController.hpp"
// *****************************************************************************


// =============================================================================
//      EMULATOR CLASS
// =============================================================================


class VirconEmulator
{
    public:
        
        // communication lines
        VirconMemoryBus  MemoryBus;
        VirconControlBus ControlBus;
        
        // hardwired motherboard components
        VirconTimer Timer;
        VirconRNG RNG;
        VirconGamepadController GamepadController;
        VirconCartridgeController CartridgeController;
        VirconMemoryCardController MemoryCardController;
        VirconNullController NullController;
        
        // components on motherboard slots
        VirconCPU CPU;
        VirconGPU GPU;
        VirconSPU SPU;
        VirconRAM RAM;
        VirconROM BiosProgramROM;
        
        // internal state
        bool PowerIsOn;
        bool Paused;
        
        // performance info (given in %)
        float LastCPULoads[ 2 ];
        float LastGPULoads[ 2 ];
        
    public:
        
        // instance handling
        VirconEmulator();
       ~VirconEmulator();
        
        // general setup
        void Initialize();
        void Terminate();
        
        // bios management
        // (bios cannot be unloaded)
        void LoadBios( const std::string& FilePath );
        
        // cartridge management
        // (only accessible when power is off)
        void LoadCartridge( const std::string& FilePath );
        void UnloadCartridge();
        
        // memory card management
        void CreateMemoryCard( const std::string& FilePath );
        void LoadMemoryCard( const std::string& FilePath );
        void UnloadMemoryCard();
        
        // general operation
        void RunNextFrame();
        void Reset();
        void PowerOn();
        void PowerOff();
        void Pause();
        void Resume();
        
        // external queries
        bool HasCartridge();
        bool HasMemoryCard();
        bool HasGamepad( int Number );
        
        // external volume control
        float GetOutputVolume();
        void SetOutputVolume( float Volume );
        bool IsMuted();
        void SetMute( bool Mute );
        
        // I/O functions
        void ProcessEvent( SDL_Event Event );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

