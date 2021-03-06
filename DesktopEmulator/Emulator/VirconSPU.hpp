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


// Our sound system uses a set of sound buffers that are
// continuously re-filled and re-queued to prevent audio
// output from ever running out of sound samples (which
// would produce clicks, silences, or terminate audio).
// Buffers can be configured in number and size within
// these limits to adapt audio timing to each system.
// Increasing the total samples will increase audio latency
// but will ensure that less capable systems have enough
// time to update audio and therefore prevent sound problems
#define MIN_BUFFERS            2
#define MAX_BUFFERS           16
#define MIN_BUFFER_SAMPLES   256
#define MAX_BUFFER_SAMPLES  4096
#define BYTES_PER_SAMPLE       4   // 1 sample = 2 channels with a 16-bit value each


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
    ChannelLoopEnabled
};

// used as limit of local port numbers
const int32_t SPU_LastPort = (int32_t)SPU_LocalPorts::ChannelLoopEnabled;


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

// translation of Ogg Vorbis error codes to text
std::string TranslateOggError( int ErrorCode );


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
        ALuint SoundBufferIDs[ MAX_BUFFERS ];
        
        // sound buffer configuration
        int NumberOfBuffers;
        int SamplesPerBuffer;
        
        // Variables for playback thread
        friend int SPUPlaybackThread( void* );
        SDL_Thread*  PlaybackThread;
        
        // variables accessed by the thread for playback control
        std::string  ThreadErrorMessage;
        bool ThreadExitFlag;           // used by the main thread to stop the playing thread
        bool ThreadPauseFlag;          // used by the main thread to hold the playing thread on pause
        bool ThreadUsingBuffers;       // set/unset by the thread to signal its buffer use sections
        
        // external volume control
        // (used not by Vircon but by the GUI)
        float OutputVolume;
        bool Mute;
        
    private:
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Internal auxiliary methods
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // generate sound to play
        bool FillSoundBuffer( ALuint BufferID );
        
        // handling playback buffer queue
        int  GetQueuedBuffers();
        int  GetProcessedBuffers();
        bool UpdateBufferQueue();
        void ClearBufferQueue();
        void FillBufferQueue();
        
        // operating the playback thread
        void LaunchPlaybackThread();
        void StopPlaybackThread();
        
        // inter-thread synchronization
        bool WaitForBufferAccess( unsigned long Milliseconds );
        
    public:
        
        // instance handling
        VirconSPU();
       ~VirconSPU();
        
        // handling audio objects
        void InitializeAudio();
        void TerminateAudio();
        
        // handling of audio resources
        void LoadSound( SPUSound& TargetSound, const std::string& FilePath );                   // from WAV file
        void LoadSound( SPUSound& TargetSound, SPUSample* Samples, unsigned NumberOfSamples );  // from samples in memory
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


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

