// *****************************************************************************
    // start include guard
    #ifndef V32EMULATOR_HPP
    #define V32EMULATOR_HPP
    
    // include project headers
    #include "V32CPU.hpp"
    #include "V32GPU.hpp"
    #include "V32SPU.hpp"
    #include "V32Timer.hpp"
    #include "V32RNG.hpp"
    #include "V32Memory.hpp"
    #include "V32GamepadController.hpp"
    #include "V32CartridgeController.hpp"
    #include "V32MemoryCardController.hpp"
    #include "V32NullController.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      EMULATOR CLASS
    // =============================================================================
    
    
    class V32Emulator
    {
        private:
            
            // communication lines
            V32MemoryBus  MemoryBus;
            V32ControlBus ControlBus;
            
            // hardwired motherboard components
            V32Timer Timer;
            V32RNG RNG;
            V32GamepadController GamepadController;
            V32CartridgeController CartridgeController;
            V32MemoryCardController MemoryCardController;
            V32NullController NullController;
            
            // components on motherboard slots
            V32CPU CPU;
            V32GPU GPU;
            V32SPU SPU;
            V32RAM RAM;
            V32ROM BiosProgramROM;
            
            // internal state
            bool PowerIsOn;
            bool Paused;
            
            // performance info (given in %)
            float LastCPULoads[ 2 ];
            float LastGPULoads[ 2 ];
            
        public:
            
            // instance handling
            V32Emulator();
           ~V32Emulator();
            
            // resource management
            void Initialize();
            void Terminate();
            void SetSPUSoundBuffers( int NumberOfBuffers );
            int GetSPUSoundBuffers();
            
            // external general operation
            void Pause();
            void Resume();
            bool IsPaused();
            
            // control signals
            void SetPower( bool On );
            void Reset();
            void RunNextFrame();
            
            // general status queries
            bool IsPowerOn();
            bool IsCPUHalted();
            float GetCPULoad();
            float GetGPULoad();
            
            // bios management
            // (bios cannot be unloaded)
            void LoadBios( const std::string& FilePath );
            
            // cartridge management
            // (only accessible when power is off)
            void LoadCartridge( const std::string& FilePath );
            void UnloadCartridge();
            bool HasCartridge();
            std::string GetCartridgeFileName();
            
            // memory card management
            void CreateMemoryCard( const std::string& FilePath );
            void LoadMemoryCard( const std::string& FilePath );
            void UnloadMemoryCard();
            void SaveMemoryCard();
            bool HasMemoryCard();
            bool WasMemoryCardModified();
            std::string GetMemoryCardFileName();
            
            // gamepad management
            void SetGamepadConnection( int GamepadPort, bool Connected );
            void SetGamepadControl( int GamepadPort, GamepadControls Control, bool Pressed );    
            bool HasGamepad( int GamepadPort );
            
            // timer management
            void SetCurrentDate( int Year, int DaysWithinYear );
            void SetCurrentTime( int Hours, int Minutes, int Seconds );
            
            // external volume control
            float GetOutputVolume();
            void SetOutputVolume( float Volume );
            bool IsMuted();
            void SetMute( bool Mute );
            
            // I/O functions
            void ProcessEvent( SDL_Event Event );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
