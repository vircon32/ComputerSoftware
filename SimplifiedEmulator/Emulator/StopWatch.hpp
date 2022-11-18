// *****************************************************************************
    // start include guard
    #ifndef STOPWATCH_HPP
    #define STOPWATCH_HPP

    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      CLASS FOR STEP-BY-STEP TIME MEASUREMENT
// =============================================================================


class StopWatch
{
    private:
        
        Uint64 CounterFrequency;     // Counter's counts per second
        Uint64 LastTimestamp;        // Last obtained count
        
    public:
        
        // class constructor
        StopWatch();
        
        // user methods
        double LastAbsoluteTime();   // last step's counter value translated to seconds
        double GetStepTime();        // duration of last time step in seconds
        double GetResolution();      // tests the minimum detectable time lapse (in seconds)
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
