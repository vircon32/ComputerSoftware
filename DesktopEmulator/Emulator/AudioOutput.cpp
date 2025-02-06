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
    #include <cstring>          // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************

    
// =============================================================================
//      AUDIO OUTPUT: INSTANCE HANDLING
// =============================================================================


AudioOutput::AudioOutput()
{
    // set null IDs for OpenAL objects
    AudioDeviceID = 0;
    
    // initial state for playback
    memset( &AudioFormat, 0, sizeof(SDL_AudioSpec) );
    memset( &PlaybackBuffer, 0, sizeof(V32::SPUOutputBuffer) );
    
    // set default configuration for sound buffers
    LatencyFrames = 3;      // audio latency would be 3 * (1/60 s) = 50 ms
    
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
    LOG( "Initializing audio output" );
    
    // tell SDL the audio format we will use
    SDL_AudioSpec DesiredAudioFormat;
    memset( &DesiredAudioFormat, 0, sizeof(SDL_AudioSpec) );
    
    DesiredAudioFormat.freq = V32::Constants::SPUSamplingRate;
    DesiredAudioFormat.format = AUDIO_S16LSB;
    DesiredAudioFormat.channels = 2;           // stereo
    DesiredAudioFormat.samples = 2048;         // must be power of 2
    DesiredAudioFormat.callback = nullptr;     // we use push, not callback
    
    // open audio device for playback
    AudioDeviceID = SDL_OpenAudioDevice
    (
        nullptr,                // default device
        false,                  // don't need recording
        &DesiredAudioFormat,    // format we want
        &AudioFormat,           // format we get 
        0                       // allow no spec changes, (SDL will convert audio when needed)
    );
    
    if( AudioDeviceID <= 0 )
      THROW( "Failed to open audio device: " + string( SDL_GetError() ) );
}

// -----------------------------------------------------------------------------

void AudioOutput::Terminate()
{
    LOG( "Terminating audio output" );
    
    // do nothing if audio was not initialized
    if( !AudioDeviceID || !IsDeviceReady() )
      return;
    
    // stop any currently playing sounds
    ClearBufferQueue();
    
    // close the audio device
    SDL_CloseAudioDevice( AudioDeviceID );
}

// =============================================================================
//      AUDIO OUTPUT: QUERYING SOUND DEVICE STATE
// =============================================================================


bool AudioOutput::IsDeviceReady()
{
    return (SDL_GetAudioDeviceStatus( AudioDeviceID ) != SDL_AUDIO_STOPPED);
}

// -----------------------------------------------------------------------------

bool AudioOutput::IsDevicePlaying()
{
    return (SDL_GetAudioDeviceStatus( AudioDeviceID ) == SDL_AUDIO_PLAYING);
}


// =============================================================================
//      AUDIO OUTPUT: BUFFER CONFIGURATION
// =============================================================================


void AudioOutput::SetLatencyFrames( int NewLatencyFrames )
{
    Clamp( LatencyFrames, MIN_LATENCY_FRAMES, MAX_LATENCY_FRAMES );
    LatencyFrames = NewLatencyFrames;
}

// -----------------------------------------------------------------------------

int AudioOutput::GetLatencyFrames()
{
    return LatencyFrames;
}


// =============================================================================
//      AUDIO OUTPUT: EXTERNAL GENERAL OPERATION
// =============================================================================


void AudioOutput::Reset()
{
    // stop any currently playing sounds
    ClearBufferQueue();
    
    // reset the output buffer
    memset( PlaybackBuffer.Samples, 0, sizeof(PlaybackBuffer.Samples) );
    
    // reinitialize audio playback
    InitializeBufferQueue();
    
    // do NOT reset output volume configuration!
}

// -----------------------------------------------------------------------------

void AudioOutput::ChangeFrame()
{
    // ensure sound is never paused while the SPU is
    // actually running (this is a fail-safe mechanism
    // to prevent the emulator from losing audio in
    // some specific window, input or file events)
    if( !IsDevicePlaying() )
      Play();
    
    // use next frame's sound
    QueueNextBuffer();
}

// -----------------------------------------------------------------------------

void AudioOutput::Pause()
{
    SDL_PauseAudioDevice( AudioDeviceID, true );
}

// -----------------------------------------------------------------------------

void AudioOutput::Play()
{
    SDL_PauseAudioDevice( AudioDeviceID, false );
}


// =============================================================================
//      AUDIO OUTPUT: EXTERNAL VOLUME CONTROL
// =============================================================================


void AudioOutput::SetOutputVolume( float Volume )
{
    OutputVolume = Volume;
    Clamp( OutputVolume, 0, 1 );
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
}

// -----------------------------------------------------------------------------

bool AudioOutput::IsMuted()
{
    return Mute;
}


// =============================================================================
//      AUDIO OUTPUT: HANDLING PLAYBACK BUFFER QUEUE
// =============================================================================


void AudioOutput::ClearBufferQueue()
{
    Pause();
    SDL_ClearQueuedAudio( AudioDeviceID );
}

// -----------------------------------------------------------------------------

void AudioOutput::InitializeBufferQueue()
{
    for( int i = 0; i < LatencyFrames; i++ )
      if( !QueueNextBuffer() )
        THROW( "Failed to initialize audio buffer queue: " + string( SDL_GetError() ) );
    
    Play();
}

// -----------------------------------------------------------------------------

// returns true on success
bool AudioOutput::QueueNextBuffer()
{
    // console sound output is ignored when muted
    if( Mute ) return true;
    
    // obtain sound output for the current frame
    Console.GetFrameSoundOutput( PlaybackBuffer );
    
    // apply external volume;
    // within SPU output volume works linearly
    // (it is just a gain level) but here we
    // will treat it quadratically to get the
    // human-perceived output volume level
    // vary in a more progressive way
    int16_t* SingleSample = (int16_t*)PlaybackBuffer.Samples;
    float QuadraticVolume = OutputVolume * OutputVolume;
    
    for( int i = 0; i < V32::Constants::SPUSamplesPerFrame; i++ )
    {
        // twice, for left and right samples
        *(SingleSample++) *= QuadraticVolume;
        *(SingleSample++) *= QuadraticVolume;
    }
    
    // queue it to play
    return !SDL_QueueAudio( AudioDeviceID, PlaybackBuffer.Samples, sizeof(PlaybackBuffer.Samples) );
}
