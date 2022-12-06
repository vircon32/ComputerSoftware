// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/Definitions.hpp"
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "VirconSPU.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
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
//      SPU PORT WRITERS TABLE
// =============================================================================


typedef void (*SPUPortWriter)( VirconSPU&, VirconWord );

// -----------------------------------------------------------------------------

// dispatch vector table for all 13 SPU ports
const SPUPortWriter SPUPortWriterTable[] =
{
    WriteSPUCommand,
    WriteSPUGlobalVolume,
    WriteSPUSelectedSound,
    WriteSPUSelectedChannel,
    WriteSPUSoundLength,
    WriteSPUSoundPlayWithLoop,
    WriteSPUSoundLoopStart,
    WriteSPUSoundLoopEnd,
    WriteSPUChannelState,
    WriteSPUChannelAssignedSound,
    WriteSPUChannelVolume,
    WriteSPUChannelSpeed,
    WriteSPUChannelLoopEnabled,
    WriteSPUChannelSpeed,
    WriteSPUChannelPosition
};


// =============================================================================
//      VIRCON SPU: INSTANCE HANDLING
// =============================================================================


VirconSPU::VirconSPU()
{
    PointedChannel = nullptr;
    PointedSound = nullptr;
    
    // set null IDs for OpenAL objects
    SoundSourceID = 0;
    
    for( int i = 0; i < MAX_BUFFERS; i++ )
      SoundBufferIDs[ 0 ] = 0;
    
    // set default configuration for sound buffers
    SamplesPerBuffer = 512;   // 512 samples at 44100Hz = 11.6 ms
    NumberOfBuffers = 4;      // 4 x 11.6 ms = 46.4 ms audio latency (< 3 frames at 60 fps)
    
    // initial state for playback variables
    PlaybackThread = nullptr;
    ThreadExitFlag = false;
    ThreadPauseFlag = false;
    ThreadUsingBuffers = false;
    
    // initial state for output volume control
    OutputVolume = 1.0;
    Mute = false;
}

// -----------------------------------------------------------------------------

VirconSPU::~VirconSPU()
{
    // release all cartridge sounds
    for( SPUSound& S: CartridgeSounds )
      UnloadSound( S );
      
    CartridgeSounds.clear();
    
    // release BIOS sound
    UnloadSound( BiosSound );
    
    // release OpenAL objects
    TerminateAudio();
}


// =============================================================================
//      VIRCON SPU: AUDIO OBJECTS HANDLING
// =============================================================================


void VirconSPU::InitializeAudio()
{
    // don't try next operations without OpenAL active
    if( !IsOpenALActive() )
      THROW( "OpenAL is not active" );
    
    // create sound buffers to alternate streaming
    alGenBuffers( NumberOfBuffers, SoundBufferIDs );
    
    // create a sound SourceID to play the buffers
    alGenSources( 1, &SoundSourceID );
    
    // configure sound source
    alSource3f( SoundSourceID, AL_POSITION, 0, 0, 0 );
    alSourcef ( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
    
    // start the playback thread
    LaunchPlaybackThread();
    
    // now, actually start playing
    FillBufferQueue();
    alSourcePlay( SoundSourceID );
}

// -----------------------------------------------------------------------------

void VirconSPU::TerminateAudio()
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
    alDeleteBuffers( NumberOfBuffers, SoundBufferIDs );
    
    // delete sound source
    alDeleteSources( 1, &SoundSourceID );
}


// =============================================================================
//      VIRCON SPU: HANDLING PLAYBACK THREAD
// =============================================================================


void VirconSPU::LaunchPlaybackThread()
{
    LOG( "Creating SPU playback thread" );
    
    // ensure thread continuity
    ThreadExitFlag     = false;
    ThreadUsingBuffers = true;
    
    // create thread, if needed
    if( !PlaybackThread )
    {
        PlaybackThread = SDL_CreateThread
        (
            SPUPlaybackThread,   // function to use as thread entry point
            "Playback",          // thread name
            this                 // function parameters (= the owner OggPlayer instance)
        );
    }
    
    if( !PlaybackThread )
      THROW( "Could not create SPU playback thread" );
}

// -----------------------------------------------------------------------------

void VirconSPU::StopPlaybackThread()
{
    LOG( "Stopping SPU playback thread" );
    
    ThreadExitFlag = true;
    
    // wait for thread to terminate (only for a limited time)
    if( PlaybackThread )
    {
        int ExitCode = 0;
        SDL_WaitThread( PlaybackThread, &ExitCode );
    }
    
    PlaybackThread = nullptr;
}

// -----------------------------------------------------------------------------

bool VirconSPU::WaitForBufferAccess( unsigned long Milliseconds )
{
    // get initial tick count
    unsigned long InitialTime = SDL_GetTicks();
    
    // wait until time expires...
    do
    {
        // ...while checking buffer availability
        if( !ThreadUsingBuffers )
        return true;
    }
    while( (SDL_GetTicks() + InitialTime) != Milliseconds );    // we do not use < to avoid errors on tick count wrapping
    
    return false;
}


// =============================================================================
//      VIRCON SPU: AUDIO RESOURCES HANDLING
// =============================================================================


void VirconSPU::LoadSound( SPUSound& TargetSound, const string& FilePath )
{
    // use OpenAL loader
    // convert buffer to Vircon format
    // now call the other version of this function
}

// -----------------------------------------------------------------------------

void VirconSPU::LoadSound( SPUSound& TargetSound, SPUSample* Samples, unsigned NumberOfSamples )
{
    // copy the buffer to target sound
    TargetSound.Samples.resize( NumberOfSamples );
    memcpy( &TargetSound.Samples[0], Samples, NumberOfSamples * 4 );
    
    // update sound length
    TargetSound.Length = NumberOfSamples;
    
    // set initial loop properties
    TargetSound.PlayWithLoop = false;
    TargetSound.LoopStart = 0;
    TargetSound.LoopEnd = TargetSound.Length - 1;
}

// -----------------------------------------------------------------------------

void VirconSPU::UnloadSound( SPUSound& TargetSound )
{
    TargetSound.Samples.clear();
    TargetSound.Length = 0;
}


// =============================================================================
//      VIRCON SPU: I/O BUS CONNECTION
// =============================================================================


bool VirconSPU::ReadPort( int32_t LocalPort, VirconWord& Result )
{
    // check range
    if( LocalPort > SPU_LastPort )
      return false;
    
    // command port is write-only
    if( LocalPort == (int32_t)SPU_LocalPorts::Command )
      return false;
    
    // CASE 1: read from SPU-level parameters
    if( LocalPort < (int32_t)SPU_LocalPorts::SoundLength )
    {
        VirconWord* SPURegisters = (VirconWord*)(&Command);
        Result = SPURegisters[ LocalPort ];
    }
    
    // CASE 2: Read from sound-level parameters
    else if( LocalPort < (int32_t)SPU_LocalPorts::ChannelState )
    {
        VirconWord* SoundRegisters = (VirconWord*)PointedSound;
        int32_t SoundPort = LocalPort - (int32_t)SPU_LocalPorts::SoundLength;
        Result = SoundRegisters[ SoundPort ];
    }
    
    // CASE 3: Read from channel-level parameters
    else
    {
        // position is a double, so we need to truncate it to its integer part
        if( LocalPort == (int32_t)SPU_LocalPorts::ChannelPosition )
          Result.AsInteger = (int32_t)PointedChannel->Position;
        
        // other channel ports can just be read as a word
        else
        {
            VirconWord* ChannelRegisters = (VirconWord*)PointedChannel;
            int32_t ChannelPort = LocalPort - (int32_t)SPU_LocalPorts::ChannelState;
            Result = ChannelRegisters[ ChannelPort ];
        }
    }
    
    return true;
}

// -----------------------------------------------------------------------------

bool VirconSPU::WritePort( int32_t LocalPort, VirconWord Value )
{
    // check range
    if( LocalPort > SPU_LastPort )
      return false;
    
    // redirect to the needed specific writer
    SPUPortWriterTable[ LocalPort ]( *this, Value );
    
    // all ports can be written to except channel state and sound length
    return (LocalPort != (int32_t)SPU_LocalPorts::ChannelState)
        && (LocalPort != (int32_t)SPU_LocalPorts::SoundLength);
}


// =============================================================================
//      VIRCON SPU: GENERAL OPERATION
// =============================================================================


void VirconSPU::ChangeFrame()
{
    // (nothing for now)
}

// -----------------------------------------------------------------------------

void VirconSPU::Reset()
{
    // reset registers
    GlobalVolume = 1.0;
    SelectedSound = -1;
    SelectedChannel = 0;
    
    // reset pointed entities
    PointedSound = &BiosSound;
    PointedChannel = &Channels[ 0 ];
    
    // reset playback variables
    ThreadExitFlag = false;
    ThreadPauseFlag = false;
    
    // reset state of all channels
    for( SPUChannel& C: Channels )
    {
        C.State = IOPortValues::SPUChannelState_Stopped;
        C.AssignedSound = -1;
        C.Volume = 0.5;
        C.Speed = 1.0;
        C.LoopEnabled = false;
        
        C.Position = 0.0;
        C.CurrentSound = &BiosSound;
    }
    
    // stop any currently playing sounds
    ThreadPauseFlag = true;
    WaitForBufferAccess( 10 );
    alSourceStop( SoundSourceID );
    ClearBufferQueue();
    
    // and reset playback
    FillBufferQueue();
    alSourcePlay( SoundSourceID );
    ThreadPauseFlag = false;
    
    // reset state of the BIOS sound
    BiosSound.PlayWithLoop = false;
    BiosSound.LoopStart = 0;
    BiosSound.LoopEnd = BiosSound.Length - 1;
    
    // reset state of all cartridge sounds
    for( SPUSound& S: CartridgeSounds )
    {
        S.PlayWithLoop = false;
        S.LoopStart = 0;
        S.LoopEnd = S.Length - 1;
    }
    
    // reset sound volume
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
    
    // do NOT reset output volume configuration!
}


// =============================================================================
//      VIRCON SPU: EXECUTION OF SPU COMMANDS
// =============================================================================


void VirconSPU::PlayChannel( SPUChannel& TargetChannel )
{
    // case 1: for a stopped channel, set the initial play
    if( TargetChannel.State == IOPortValues::SPUChannelState_Stopped )
    {
        TargetChannel.Position = 0;
        TargetChannel.LoopEnabled = TargetChannel.CurrentSound->PlayWithLoop;
    }
    
    // case 2: if channel was already playing, make it retrigger
    // (same actions as case 1, but keep it separate)
    else if( TargetChannel.State == IOPortValues::SPUChannelState_Playing )
    {
        TargetChannel.Position = 0;
        TargetChannel.LoopEnabled = TargetChannel.CurrentSound->PlayWithLoop;
    }
    
    // case 3: if channel was paused, just make it resume
    // (nothing else to do)
    
    // finally, update channel state
    TargetChannel.State = IOPortValues::SPUChannelState_Playing;
}

// -----------------------------------------------------------------------------

void VirconSPU::PauseChannel( SPUChannel& TargetChannel )
{
    TargetChannel.State = IOPortValues::SPUChannelState_Paused;
}

// -----------------------------------------------------------------------------

void VirconSPU::StopChannel( SPUChannel& TargetChannel )
{
    TargetChannel.State = IOPortValues::SPUChannelState_Stopped;
    
    // when stopping, rewind sound
    // (but keep sound and configuration)
    TargetChannel.Position = 0;
}

// -----------------------------------------------------------------------------

void VirconSPU::PauseAllChannels()
{
    for( int i = 0; i < Constants::SPUSoundChannels; i++ )
      if( Channels[ i ].State == IOPortValues::SPUChannelState_Playing )
        PauseChannel( Channels[ i ] );
}

// -----------------------------------------------------------------------------

void VirconSPU::ResumeAllChannels()
{
    for( int i = 0; i < Constants::SPUSoundChannels; i++ )
      if( Channels[ i ].State == IOPortValues::SPUChannelState_Paused )
        PlayChannel( Channels[ i ] );
}

// -----------------------------------------------------------------------------

void VirconSPU::StopAllChannels()
{
    for( int i = 0; i < Constants::SPUSoundChannels; i++ )
      if( Channels[ i ].State != IOPortValues::SPUChannelState_Stopped )
        StopChannel( Channels[ i ] );
}


// =============================================================================
//      VIRCON SPU: GENERATING SOUND
// =============================================================================


// returns true if successful
bool VirconSPU::FillSoundBuffer( ALuint BufferID )
{
    // local buffer data we will work with
    static SPUSample NewSamples[ MAX_BUFFER_SAMPLES ];
    
    // determine the value for each sample in the buffer
    for( int s = 0; s < SamplesPerBuffer; s++ )
    {
        // use a local variable for speed
        SPUSample ThisSample = {0,0};
        
        // generate sound for all channels
        for( int c = 0; c < Constants::SPUSoundChannels; c++ )
        {
            // process only playing channels
            SPUChannel* ThisChannel = &Channels[ c ];
            
            if( ThisChannel->State != IOPortValues::SPUChannelState_Playing )
              continue;
            
            // pick sample at this position
            SPUSample PickedSample = ThisChannel->CurrentSound->Samples[ (int)ThisChannel->Position ];
            
            // mix the sample
            float TotalVolume = GlobalVolume * ThisChannel->Volume;
            ThisSample.LeftSample  += TotalVolume * PickedSample.LeftSample;
            ThisSample.RightSample += TotalVolume * PickedSample.RightSample;
            
            // advance at current speed
            double PreviousPosition = ThisChannel->Position;
            ThisChannel->Position += ThisChannel->Speed;
            
            // if loop is enabled, check for loop boundary
            if( ThisChannel->LoopEnabled )
            {
                int32_t LoopStart = ThisChannel->CurrentSound->LoopStart;
                int32_t LoopEnd   = ThisChannel->CurrentSound->LoopEnd;
                
                // cannot perform loop with a bad loop configuration!
                // (otherwise, fmod may throw an exception)
                if( LoopEnd > LoopStart )
                  if( PreviousPosition <= LoopEnd && ThisChannel->Position > LoopEnd )
                  {
                      // don't just go back to start: for high playback speeds we
                      // may have overshot the end position, so compensate the excess
                      double PartialAdvance = fmod( ThisChannel->Position - LoopStart, LoopEnd - LoopStart );
                      ThisChannel->Position = LoopStart + PartialAdvance;
                  }
            }
            
            // if the sound ends, stop the channel
            if( ThisChannel->Position > (ThisChannel->CurrentSound->Length - 1) )
              StopChannel( *ThisChannel );
        }
        
        NewSamples[ s ] = ThisSample;
    }
    
    // ignore OpenAL errors so far
    alGetError();
    
    // copy our local buffer to internal OpenAL one
    ALsizei BufferBytes = SamplesPerBuffer * BYTES_PER_SAMPLE;
    alBufferData( BufferID, AL_FORMAT_STEREO16, NewSamples, BufferBytes, Constants::SPUSamplingRate );
    
    return (alGetError() == AL_NO_ERROR);
}


// =============================================================================
//      VIRCON SPU: HANDLING PLAYBACK BUFFER QUEUE
// =============================================================================


int VirconSPU::GetQueuedBuffers()
{
    int QueuedBuffers = 0;
    alGetSourcei( SoundSourceID, AL_BUFFERS_QUEUED, &QueuedBuffers );
    
    return max( QueuedBuffers, 0 );
}

// -----------------------------------------------------------------------------

// NOTE: read the documentation for all cases regarding AL_BUFFERS_PROCESSED
// (will only work right with source state AL_PLAYING)
int VirconSPU::GetProcessedBuffers()
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

// this function is only called from the playback thread
// returns true if queue was updated successfully
bool VirconSPU::UpdateBufferQueue()
{
    // state validations
    if( !GetQueuedBuffers() )
      return false;
    
    // query number of queued buffers already processed
    int ProcessedBuffers = GetProcessedBuffers();
    
    // for each processed buffer: unqueue->refresh->requeue
    while( ProcessedBuffers-- )
    {
        // (1) remove the oldest processed buffer from the queue
        // NOTE: will cause errors when used in non processed buffer
        ALuint ProcessedBufferID = 0;
        alSourceUnqueueBuffers( SoundSourceID, 1, &ProcessedBufferID );
        
        // (2) fill it again with new data from the file
        if( !FillSoundBuffer( ProcessedBufferID ) )
          return false;
        
        // (3) reinsert in queue as pending
        alSourceQueueBuffers( SoundSourceID, 1, &ProcessedBufferID );
    }
    
    return true;
}

// -----------------------------------------------------------------------------

// NOTE: read the documentation for alSourceUnqueueBuffers
// (will only work right with source state AL_STOPPED)
void VirconSPU::ClearBufferQueue()
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
    }
    
    catch( const exception& e )
    {
        cout << "[Exception caught]: " << e.what() << endl;
        cout.flush();
    }
}

// -----------------------------------------------------------------------------

void VirconSPU::FillBufferQueue()
{
    // do nothing to do if a queue already exists
    if( GetQueuedBuffers() )
      return;
    
    // fill all buffers with audio to play
    // (throw when buffers could not be filled)
    for( int i = 0; i < NumberOfBuffers; i++ )
      if( !FillSoundBuffer( SoundBufferIDs[ i ] ) )
        THROW( "Cannot fill sound buffers" );
    
    // put them in the source play queue 
    alSourceQueueBuffers( SoundSourceID, NumberOfBuffers, SoundBufferIDs );
}


// =============================================================================
//      VIRCON SPU: OUTPUT VOLUME CONFIGURATION
// =============================================================================


void VirconSPU::SetOutputVolume( float NewVolume )
{
    OutputVolume = NewVolume;
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
}

// -----------------------------------------------------------------------------

void VirconSPU::SetMute( bool NewMute )
{
    Mute = NewMute;
    alSourcef( SoundSourceID, AL_GAIN, (Mute? 0 : OutputVolume) );
}
