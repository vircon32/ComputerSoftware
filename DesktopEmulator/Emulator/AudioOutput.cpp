// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "AudioOutput.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <climits>          // [ ANSI C ] Numeric limits
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************

    
// =============================================================================
//      AUXILIARY AUDIO FUNCTIONS
// =============================================================================


bool IsOpenALActive()
{
    // STEP 1: Check there is an audio context
    ALCcontext* AudioContext = alcGetCurrentContext();
    
    if( !AudioContext )
      return false;
    
    alGetError();
    
    // STEP 2: Check there is an audio device
    ALCdevice* AudioDevice = alcGetContextsDevice( AudioContext );
    
    if( !AudioDevice )
      return false;
    
    alGetError();
    
    return true;
}

// -----------------------------------------------------------------------------

ALenum GetSourceState( ALuint SourceID )
{
    ALenum State;
    alGetSourcei( SourceID, AL_SOURCE_STATE, &State );
    
    return State;
}

// -----------------------------------------------------------------------------

bool IsSourcePlaying( ALuint SourceID )
{
    return (GetSourceState( SourceID ) == AL_PLAYING);
}


// =============================================================================
//      AUDIO OUTPUT: INSTANCE HANDLING
// =============================================================================


AudioOutput::AudioOutput()
{
    // set null IDs for OpenAL objects
    SoundSourceID = 0;
    
    // initial state for buffers
    for( int i = 0; i < MAX_BUFFERS; i++ )
      PlaybackBuffers[ i ].BufferID = 0;
    
    // set default configuration for sound buffers
    NumberOfBuffers = 6;      // audio latency would be (6 / 2) * (1/60 s) = 50 ms
    
    // initial state for playback variables
    PlaybackThread = nullptr;
    ThreadExitFlag = false;
    ThreadPauseFlag = true;
    
    // initial state for output volume control
    OutputVolume = 1.0;
    Mute = false;
}

// -----------------------------------------------------------------------------

AudioOutput::~AudioOutput()
{
    // release OpenAL objects
    Terminate();
}


// =============================================================================
//      AUDIO OUTPUT: AUDIO OBJECTS HANDLING
// =============================================================================


void AudioOutput::Initialize()
{
    // don't try next operations without OpenAL active
    if( !IsOpenALActive() )
      THROW( "OpenAL is not active" );
    
    // create sound buffers to alternate streaming
    for( int i = 0; i < MAX_BUFFERS; i++ )
      alGenBuffers( 1, &PlaybackBuffers[ i ].BufferID );
    
    // create a sound SourceID to play the buffers
    alGenSources( 1, &SoundSourceID );
    
    // configure sound source
    alSource3f( SoundSourceID, AL_POSITION, 0, 0, 0 );
    alSourcef ( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
    
    // start the playback thread
    LaunchPlaybackThread();
}

// -----------------------------------------------------------------------------

void AudioOutput::Terminate()
{
    // do nothing if audio was not initialized
    if( !SoundSourceID )
      return;
    
    // don't try next operations without OpenAL active
    if( !IsOpenALActive() )
      return;
    
    // stop sound emission
    // (must be done before clearing queue or unattaching buffer)
    alSourceStop( SoundSourceID );
    
    // remove any pending queued buffers
    ClearBufferQueue();
    
    // unattach any buffer from the source
    alSourcei( SoundSourceID, AL_BUFFER, 0 );
    
    // stop the playback thread
    StopPlaybackThread();
    
    // delete sound buffers
    for( int i = 0; i < MAX_BUFFERS; i++ )
      alDeleteBuffers( 1, &PlaybackBuffers[ i ].BufferID );
    
    // delete sound source
    alDeleteSources( 1, &SoundSourceID );
}

// =============================================================================
//      AUDIO OUTPUT: BUFFER CONFIGURATION
// =============================================================================


void AudioOutput::SetNumberOfBuffers( int NewNumberOfBuffers )
{
    NumberOfBuffers = NewNumberOfBuffers;
}

// -----------------------------------------------------------------------------

int AudioOutput::GetNumberOfBuffers()
{
    return NumberOfBuffers;
}


// =============================================================================
//      AUDIO OUTPUT: EXTERNAL GENERAL OPERATION
// =============================================================================


void AudioOutput::Reset()
{
    // reset playback variables
    ThreadExitFlag = false;
    ThreadPauseFlag = true;
    
    // stop any currently playing sounds
    alSourceStop( SoundSourceID );
    ClearBufferQueue();
    
    // reset all output buffers
    for( int i = 0; i < MAX_BUFFERS; i++ )
    {
        memset( PlaybackBuffers[ i ].Contents.Samples, 0, Constants::SPUSamplesPerFrame * 4 );
        PlaybackBuffers[ i ].State = SoundBufferStates::ToBeFilled;
        PlaybackBuffers[ i ].Contents.SequenceNumber = 0;
    }
    
    // reinitialize audio playback
    InitializeBufferQueue();
    ThreadPauseFlag = false;
    alSourcePlay( SoundSourceID );
    
    // reset sound volume
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
    
    // do NOT reset output volume configuration!
}

// -----------------------------------------------------------------------------

void AudioOutput::ChangeFrame()
{
    // ensure sound is never paused while the SPU is
    // actually running (this is a fail-safe mechanism
    // to prevent the emulator from losing audio in
    // some specific window, input or file events)
    if( !IsSourcePlaying( SoundSourceID ) )
      alSourcePlay( SoundSourceID );
    
    if( ThreadPauseFlag )
      ThreadPauseFlag = false;
    
    // use next frame's sound
    FillNextSoundBuffer();
}

// -----------------------------------------------------------------------------

void AudioOutput::Pause()
{
    ThreadPauseFlag = true;
    alSourcePause( SoundSourceID );
}

// -----------------------------------------------------------------------------

void AudioOutput::Resume()
{
    // do nothing when not applicable
    if( !ThreadPauseFlag ) return;
    
    // take resume actions
    alSourcePlay( SoundSourceID );
    ThreadPauseFlag = false;
}


// =============================================================================
//      AUDIO OUTPUT: EXTERNAL VOLUME CONTROL
// =============================================================================


void AudioOutput::SetOutputVolume( float Volume )
{
    OutputVolume = Volume;
    Clamp( OutputVolume, 0, 1 );
    
    // within SPU output volume works linearly
    // (it is just a gain level) but here we
    // will treat it quadratically to get the
    // human-perceived output volume level
    // vary in a more progressive way
    float QuadraticVolume = OutputVolume * OutputVolume;
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : QuadraticVolume) );
}

// -----------------------------------------------------------------------------

float AudioOutput::GetOutputVolume()
{
    return OutputVolume;
}

// -----------------------------------------------------------------------------

void AudioOutput::SetMute( bool NewMute )
{
    Mute = NewMute;
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
}

// -----------------------------------------------------------------------------

bool AudioOutput::IsMuted()
{
    return Mute;
}


// =============================================================================
//      AUDIO OUTPUT: GENERATING SOUND
// =============================================================================


// this function is only called from the main thread;
// returns true if successful
bool AudioOutput::FillNextSoundBuffer()
{
    SoundBuffer* Buffer = FindNextBufferToFill();
    if( !Buffer ) return false;
    
    // obtain sound output for the current frame
    Console.GetFrameSoundOutput( Buffer->Contents );
    
    // ignore OpenAL errors so far
    alGetError();
    
    // copy our local buffer to internal OpenAL one
    alBufferData( Buffer->BufferID, AL_FORMAT_STEREO16, Buffer->Contents.Samples, Constants::SPUSamplesPerFrame * 4, Constants::SPUSamplingRate );
    
    // finally, change buffer state
    Buffer->State = SoundBufferStates::Filled;
    return (alGetError() == AL_NO_ERROR);
}


// =============================================================================
//      AUDIO OUTPUT: SEARCHING FOR SOUND BUFFERS
// =============================================================================


SoundBuffer& AudioOutput::FindBufferFromID( ALuint TargetID )
{
    for( int i = 0; i < NumberOfBuffers; i++ )
      if( PlaybackBuffers[ i ].BufferID == TargetID )
        return PlaybackBuffers[ i ];
    
    THROW( "Cannot find OpenAL buffer ID in SPU output buffers" );
}

// -----------------------------------------------------------------------------

SoundBuffer* AudioOutput::FindNextBufferToPlay()
{
    int MinimumSequenceNumber = INT_MAX;
    SoundBuffer* NextBufferToPlay = nullptr;
    
    for( int i = 0; i < NumberOfBuffers; i++ )
      if( PlaybackBuffers[ i ].State == SoundBufferStates::Filled )
        if( PlaybackBuffers[ i ].Contents.SequenceNumber <= MinimumSequenceNumber )
        {
            MinimumSequenceNumber = PlaybackBuffers[ i ].Contents.SequenceNumber;
            NextBufferToPlay = &PlaybackBuffers[ i ];
        }
    
    return NextBufferToPlay;
}

// -----------------------------------------------------------------------------

SoundBuffer* AudioOutput::FindNextBufferToFill()
{
    // we can refill buffers in any order, so we don't
    // need to look for the lowest sequence number here
    for( int i = 0; i < NumberOfBuffers; i++ )
      if( PlaybackBuffers[ i ].State == SoundBufferStates::ToBeFilled )
        return &PlaybackBuffers[ i ];
    
    return nullptr;
}


// =============================================================================
//      AUDIO OUTPUT: HANDLING PLAYBACK BUFFER QUEUE
// =============================================================================


int AudioOutput::GetQueuedBuffers()
{
    int QueuedBuffers = 0;
    alGetSourcei( SoundSourceID, AL_BUFFERS_QUEUED, &QueuedBuffers );
    
    return max( QueuedBuffers, 0 );
}

// -----------------------------------------------------------------------------

// NOTE: read the documentation for all cases regarding AL_BUFFERS_PROCESSED
// (will only work right with source state AL_PLAYING)
int AudioOutput::GetProcessedBuffers()
{
    // without this check, the playback thread produces "invalid operation" error on exit
    int SourceState;
    alGetSourcei( SoundSourceID, AL_SOURCE_STATE, &SourceState );
    
    if( SourceState != AL_PLAYING )
      return 0;
    
    // now do the actual check for buffers
    int ProcessedBuffers = 0;
    alGetSourcei( SoundSourceID, AL_BUFFERS_PROCESSED, &ProcessedBuffers );
    
    return max( ProcessedBuffers, 0 );
}

// -----------------------------------------------------------------------------

// NOTE: read the documentation for alSourceUnqueueBuffers
// (will only work right with source state AL_STOPPED)
void AudioOutput::ClearBufferQueue()
{
    int SourceState;
    alGetSourcei( SoundSourceID, AL_SOURCE_STATE, &SourceState );
    
    if( SourceState != AL_STOPPED )
      return;
    
    // remove all buffers from the queue
    try
    {
        // obtain the number of buffers queued for play in the source
        int QueuedBuffers = GetQueuedBuffers();
        
        // remove each buffer
        while( QueuedBuffers-- )
        {
            ALuint QueuedBufferID = 0;
            alSourceUnqueueBuffers( SoundSourceID, 1, &QueuedBufferID );
        }
        
        // update state for all buffers
        for( int i = 0; i < NumberOfBuffers; i++ )
          PlaybackBuffers[ i ].State = SoundBufferStates::ToBeFilled;
    }
    
    catch( const exception& e )
    {
        LOG( "Exception clearing sound buffer queue: " + string(e.what()) );
    }
}

// -----------------------------------------------------------------------------

void AudioOutput::InitializeBufferQueue()
{
    // we will only fill half of the buffers
    // (the rest are left free for later use)
    for( int i = 0; i < NumberOfBuffers / 2 ; i++ )
      FillNextSoundBuffer();
    
    // as an exception, this one time we will
    // need to call this from the main thread;
    // but it is still safe: at this point the
    // playback thread is paused and the sound
    // source is stopped 
    QueueFilledBuffers();
}

// -----------------------------------------------------------------------------

// this function is only called from the playback thread,
// with only one exception for initial queuing on reset
void AudioOutput::QueueFilledBuffers()
{
    while( true )
    {
        SoundBuffer* BufferToPlay = FindNextBufferToPlay();
        if( !BufferToPlay ) return;
        
        // put them in the source play queue 
        alSourceQueueBuffers( SoundSourceID, 1, &BufferToPlay->BufferID );
        BufferToPlay->State = SoundBufferStates::QueuedToPlay;
    }
}

// -----------------------------------------------------------------------------

// this function is only called from the playback thread
void AudioOutput::UnqueuePlayedBuffers()
{
    // state validations
    if( !GetQueuedBuffers() )
      return;
    
    // query number of queued buffers already processed
    int ProcessedBuffers = GetProcessedBuffers();
    
    // unqueue every processed buffer
    while( ProcessedBuffers-- )
    {
        ALuint ProcessedBufferID = 0;
        alSourceUnqueueBuffers( SoundSourceID, 1, &ProcessedBufferID );
        
        // buffer is ready to be refilled
        SoundBuffer& UnqueuedBuffer = FindBufferFromID( ProcessedBufferID );
        UnqueuedBuffer.State = SoundBufferStates::ToBeFilled;
    }
}


// =============================================================================
//      AUDIO OUTPUT: HANDLING PLAYBACK THREAD
// =============================================================================


void AudioOutput::LaunchPlaybackThread()
{
    LOG( "Creating audio playback thread" );
    
    // ensure thread continuity, but don't play yet
    ThreadExitFlag = false;
    ThreadPauseFlag = true;
    
    // create thread, if needed
    if( !PlaybackThread )
    {
        PlaybackThread = SDL_CreateThread
        (
            AudioPlaybackThread,   // function to use as thread entry point
            "Playback",            // thread name
            this                   // function parameters (= the owner instance)
        );
    }
    
    if( !PlaybackThread )
      THROW( "Could not create audio playback thread" );
}

// -----------------------------------------------------------------------------

void AudioOutput::StopPlaybackThread()
{
    LOG( "Stopping audio playback thread" );
    
    ThreadExitFlag = true;
    
    // wait for thread to terminate (only for a limited time)
    if( PlaybackThread )
    {
        int ExitCode = 0;
        SDL_WaitThread( PlaybackThread, &ExitCode );
    }
    
    PlaybackThread = nullptr;
}
