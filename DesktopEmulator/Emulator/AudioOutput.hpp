// *****************************************************************************
    // start include guard
    #ifndef AUDIOOUTPUT_HPP
    #define AUDIOOUTPUT_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <string>		    // [ C++ STL ] Strings
    
    // include OpenAL headers
    #if defined(__APPLE__)
      #include <OpenAL/al.h>    // [ OpenAL ] Main header
      #include <OpenAL/alc.h>   // [ OpenAL ] Audio contexts
    #else
      #include <AL/al.h>        // [ OpenAL ] Main header
      #include <AL/alc.h>       // [ OpenAL ] Audio contexts
    #endif
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>       // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR SOUND BUFFERS
// =============================================================================


// Our sound system uses a set of sound buffers that are
// continuously re-filled and re-queued to prevent audio
// output from ever running out of sound samples (which
// would produce clicks, silences, or terminate audio).
// Buffers can be configured in number within these limits
// to adapt audio timing to each system. Adding buffers
// will increase audio latency but will ensure that less
// capable systems have enough time to update audio and
// therefore prevent sound problems.

#define MIN_BUFFERS            4
#define MAX_BUFFERS           16

// -----------------------------------------------------------------------------

// We will use these 3 states to handle audio buffers.
// Thread safety is achieved by making each thread act
// only over different sets of buffer states, so they
// can never be operating over the same buffer at once.
enum class SoundBufferStates
{
    ToBeFilled,     // only handled by the main thread
    Filled,         // only handled by the playback thread
    QueuedToPlay    // only handled by the playback thread
};

// -----------------------------------------------------------------------------

// define the sound buffers used for SPU audio output;
// buffers contain 1 frame of audio = 44100 / 60 samples
typedef struct
{
    ALuint BufferID;
    SoundBufferStates State;
    V32::SPUOutputBuffer Contents;
}
SoundBuffer;


// =============================================================================
//      AUXILIARY AUDIO FUNCTIONS
// =============================================================================

// checking that OpenAL is active
bool IsOpenALActive();

// source properties
ALenum GetSourceState( ALuint SourceID );
bool IsSourcePlaying ( ALuint SourceID );


// =============================================================================
//      FUNCTIONS EXTERNAL TO THE AUDIO CLASS
// =============================================================================


// thread function for background continuous play
int AudioPlaybackThread( void* Parameters );


// =============================================================================
//      AUDIO OUTPUT HANDLER CLASS
// =============================================================================


class AudioOutput
{
    public:
        
        // OpenAL mixer objects
        ALuint SoundSourceID;
        
        // sound buffer configuration
        int NumberOfBuffers;
        SoundBuffer PlaybackBuffers[ MAX_BUFFERS ];
    
        // Variables for playback thread
        friend int AudioPlaybackThread( void* );
        SDL_Thread* PlaybackThread;
        
        // variables accessed by the thread for playback control
        std::string ThreadErrorMessage;     // used by the playback thread to report errors on exceptions
        bool ThreadExitFlag;                // used by the main thread to stop the playing thread
        bool ThreadPauseFlag;               // used by the main thread to hold the playing thread on pause
        
        // external volume control
        float OutputVolume;
        bool Mute;
        
    private:
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Internal auxiliary methods
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // generate sound to play
        bool FillNextSoundBuffer();
        
        // searching for sound buffers
        SoundBuffer& FindBufferFromID( ALuint TargetID );
        SoundBuffer* FindNextBufferToPlay();
        SoundBuffer* FindNextBufferToFill();
        
        // handling playback buffer queue
        int GetQueuedBuffers();
        int GetProcessedBuffers();
        void UnqueuePlayedBuffers();
        void QueueFilledBuffers();
        void ClearBufferQueue();
        void InitializeBufferQueue();
        
        // operating the playback thread
        void LaunchPlaybackThread();
        void StopPlaybackThread();
        
    public:
        
        // instance handling
        AudioOutput();
       ~AudioOutput();
        
        // resource management
        void Initialize();
        void Terminate();
        
        // buffer configuration
        void SetNumberOfBuffers( int NewNumberOfBuffers );
        int GetNumberOfBuffers();
        
        // external general operation
        void Reset();
        void ChangeFrame();
        void Pause();
        void Resume();
        
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
