// *****************************************************************************
    // start include guard
    #ifndef VIRCONSPU_HPP
    #define VIRCONSPU_HPP
    
    // include common vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Definitions.hpp"
    
    // include project headers
    #include "VirconBuses.hpp"
    
    // include OpenAL headers
    #if defined(__APPLE__)
      #include <OpenAL/al.h>      // [ OpenAL ] Main header
      #include <OpenAL/alc.h>     // [ OpenAL ] Audio contexts
    #else
      #include <AL/al.h>          // [ OpenAL ] Main header
      #include <AL/alc.h>         // [ OpenAL ] Audio contexts
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
#define BUFFER_SAMPLES       735   // buffers contain 1 frame of audio = 44100 / 60 samples
#define BYTES_PER_SAMPLE       4   // 1 sample = 2 channels with a 16-bit value each
#define BYTES_PER_BUFFER    2940   // 735 samples * 4 bytes/sample

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

// define the sound buffers used for SPU audio output
typedef struct
{
    ALuint BufferID;
    int32_t SequenceNumber;
    SoundBufferStates State;
    SPUSample Samples[ BUFFER_SAMPLES ];
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
//      SPU DEFINITIONS
// =============================================================================


// LOCAL port numbers!
// (to be added the initial address)
enum class SPU_LocalPorts: int32_t
{
    Command = 0,
    
    GlobalVolume,
    SelectedSound,
    SelectedChannel,
    
    SoundLength,
    SoundPlayWithLoop,
    SoundLoopStart,
    SoundLoopEnd,
    
    ChannelState,
    ChannelAssignedSound,
    ChannelVolume,
    ChannelSpeed,
    ChannelLoopEnabled,
    ChannelPosition
};

// used as limit of local port numbers
const int32_t SPU_LastPort = (int32_t)SPU_LocalPorts::ChannelPosition;


// =============================================================================
//      SPU DATA STRUCTURES
// =============================================================================


typedef struct
{
    // accesible ports
    int32_t Length;
    int32_t PlayWithLoop;
    int32_t LoopStart;
    int32_t LoopEnd;
    
    // actual sound samples
    std::vector< SPUSample > Samples;
}
SPUSound;

// -----------------------------------------------------------------------------

typedef struct
{
    IOPortValues State;
    int32_t AssignedSound;
    float Volume;
    float Speed;
    int32_t LoopEnabled;
    
    // other needed fields
    double Position;         // we need double for extra range and precision
    SPUSound* CurrentSound;  // working pointer for speed
}
SPUChannel;


// =============================================================================
//      FUNCTIONS EXTERNAL TO THE SPU
// =============================================================================


// thread function for background continuous play
int SPUPlaybackThread( void* Parameters );


// =============================================================================
//      VIRCON SPU CLASS
// =============================================================================


class VirconSPU: public VirconControlInterface
{
    public:
        
        // sounds loaded into SPU
        SPUSound BiosSound;
        std::vector< SPUSound > CartridgeSounds;
        
        // SPU registers
        int32_t Command;
        float GlobalVolume;
        int32_t SelectedSound;
        int32_t SelectedChannel;
        
        // accessors to active entities
        SPUSound*   PointedSound;
        SPUChannel* PointedChannel;
        
        // sound channels
        SPUChannel Channels[ Constants::SPUSoundChannels ];
        
        // OpenAL mixer objects
        ALuint SoundSourceID;
        
        // sound buffer configuration
        int NumberOfBuffers;
        SoundBuffer OutputBuffers[ MAX_BUFFERS ];
        int NextBufferSequenceNumber;
        
        // Variables for playback thread
        friend int SPUPlaybackThread( void* );
        SDL_Thread*  PlaybackThread;
        
        // variables accessed by the thread for playback control
        std::string  ThreadErrorMessage;    // used by the playback thread to report errors on exceptions
        bool ThreadExitFlag;                // used by the main thread to stop the playing thread
        bool ThreadPauseFlag;               // used by the main thread to hold the playing thread on pause
        
        // external volume control
        // (used not by Vircon but by the GUI)
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
        VirconSPU();
       ~VirconSPU();
        
        // handling audio objects
        void InitializeAudio();
        void TerminateAudio();
        
        // handling of audio resources
        void LoadSound( SPUSound& TargetSound, SPUSample* Samples, unsigned NumberOfSamples );
        void UnloadSound( SPUSound& TargetSound );
        
        // I/O bus connection
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
        // general operation
        void ChangeFrame();
        void Reset();
        
        // execution of GPU commands
        void PlayChannel ( SPUChannel& TargetChannel );
        void PauseChannel( SPUChannel& TargetChannel );
        void StopChannel ( SPUChannel& TargetChannel );
        void PauseAllChannels();
        void ResumeAllChannels();
        void StopAllChannels();
        
        // output volume configuration
        void SetMute( bool NewMute );
        void SetOutputVolume( float NewVolume );
};


// =============================================================================
//      SPU REGISTER WRITERS
// =============================================================================


void WriteSPUCommand              ( VirconSPU& SPU, VirconWord Value );
void WriteSPUGlobalVolume         ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSelectedSound        ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSelectedChannel      ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSoundLength          ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSoundPlayWithLoop    ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSoundLoopStart       ( VirconSPU& SPU, VirconWord Value );
void WriteSPUSoundLoopEnd         ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelState         ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelAssignedSound ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelVolume        ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelSpeed         ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelLoopEnabled   ( VirconSPU& SPU, VirconWord Value );
void WriteSPUChannelPosition      ( VirconSPU& SPU, VirconWord Value );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

