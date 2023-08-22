// *****************************************************************************
    // start include guard
    #ifndef V32CONSOLE_HPP
    #define V32CONSOLE_HPP
    
    // include console logic headers
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
    
    // include C/C++ headers
    #include <string>         // [ C++ STL ] Strings
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CONSOLE CLASS
    // =============================================================================
    
    
    class V32Console
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
            
            // performance info (given in %)
            float LastCPULoads[ 2 ];
            float LastGPULoads[ 2 ];
            
        public:
            
            // instance handling
            V32Console();
           ~V32Console();
            
            // - - - - - - - - - - - - - - - - - - - - - - - -
            //   EXTERNAL INTERFACES: API FUNCTIONS
            // - - - - - - - - - - - - - - - - - - - - - - - -
            
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
            // (bios cannot be unloaded, but some implementations may need it)
            void LoadBios( const std::string& FilePath );
            void UnloadBios();
            bool HasBios();
            
            // cartridge management
            // (only accessible when power is off)
            void LoadCartridge( const std::string& FilePath );
            void UnloadCartridge();
            bool HasCartridge();
            std::string GetCartridgeFileName();
            std::string GetCartridgeTitle();
            
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
            
            // sound output management
            void GetFrameSoundOutput( SPUOutputBuffer& OutputBuffer );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
