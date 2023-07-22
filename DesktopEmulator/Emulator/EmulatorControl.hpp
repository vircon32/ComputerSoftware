// *****************************************************************************
    // start include guard
    #ifndef EMULATORCONTROL_HPP
    #define EMULATORCONTROL_HPP
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>       // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      CLASS FOR EMULATOR CENTRAL CONTROL
// =============================================================================


class EmulatorControl
{
    private:
        
        bool Paused;
    
    public:
        
        // instance handling
        EmulatorControl();
       ~EmulatorControl();
        
        // resource handling
        void Initialize();
        void Terminate();
        
        // general operation
        void Pause();
        void Resume();
        bool IsPaused();

        void SetPower( bool On );
        bool IsPowerOn();
        void Reset();
        void RunNextFrame();

        //void Emulator_Update( float DeltaTime );
        void ProcessEvent( SDL_Event Event );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
