// *****************************************************************************
    // start include guard
    #ifndef AUDIOOUTPUT_HPP
    #define AUDIOOUTPUT_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR SOUND BUFFERS
// =============================================================================


// Our sound system uses a 1-frame sound buffer that is
// continuously re-filled and re-queued to prevent audio
// output from ever running out of sound samples (which
// would produce clicks, silences, or terminate audio).
// An given number of frames (latency) will initially
// be queued so that playback is able to adapt to some
// variability in frame timing. Adding latency frames
// will increase audio delay but will ensure that less
// capable systems have enough time to update audio and
// therefore prevent sound problems.

#define MIN_LATENCY_FRAMES     2
#define MAX_LATENCY_FRAMES    12


// =============================================================================
//      AUDIO OUTPUT HANDLER CLASS
// =============================================================================


class AudioOutput
{
    public:
        
        // SDL audio objects
        SDL_AudioSpec AudioFormat;
        SDL_AudioDeviceID AudioDeviceID;
        
        // frame based playback
        int LatencyFrames;
        V32::SPUOutputBuffer PlaybackBuffer;
        
        // external volume control
        float OutputVolume;
        bool Mute;
        
    private:
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Internal auxiliary methods
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // handling playback buffer queue
        void ClearBufferQueue();
        void InitializeBufferQueue();
        bool QueueNextBuffer();
        
        // querying sound device state
        bool IsDeviceReady();
        bool IsDevicePlaying();
        
    public:
        
        // instance handling
        AudioOutput();
       ~AudioOutput();
        
        // resource management
        void Initialize();
        void Terminate();
        
        // buffer configuration
        void SetLatencyFrames( int NewLatencyFrames );
        int GetLatencyFrames();
        
        // external general operation
        void Reset();
        void ChangeFrame();
        void Pause();
        void Play();
        
        // external volume control
        float GetOutputVolume();
        void SetOutputVolume( float Volume );
        bool IsMuted();
        void SetMute( bool Mute );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
