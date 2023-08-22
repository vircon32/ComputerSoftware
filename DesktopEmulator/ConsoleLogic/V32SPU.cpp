// *****************************************************************************
    // include console logic headers
    #include "V32SPU.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    #include <cmath>            // [ ANSI C ] Math
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      SPU PORT WRITERS TABLE
    // =============================================================================
    
    
    typedef bool (*SPUPortWriter)( V32SPU&, V32Word );
    
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
    //      V32 SPU: INSTANCE HANDLING
    // =============================================================================
    
    
    V32SPU::V32SPU()
    {
        PointedChannel = nullptr;
        PointedSound = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    V32SPU::~V32SPU()
    {
        // don't release any sounds
        // (this is done at console destructor)
    }
    
    
    // =============================================================================
    //      V32 SPU: AUDIO RESOURCES HANDLING
    // =============================================================================
    
    
    void V32SPU::LoadSound( SPUSound& TargetSound, SPUSample* Samples, unsigned NumberOfSamples )
    {
        // copy the buffer to target sound
        TargetSound.Samples.resize( NumberOfSamples );
        memcpy( &TargetSound.Samples[ 0 ], Samples, NumberOfSamples * 4 );
        
        // update sound length
        TargetSound.Length = NumberOfSamples;
        
        // set initial loop properties
        TargetSound.PlayWithLoop = false;
        TargetSound.LoopStart = 0;
        TargetSound.LoopEnd = TargetSound.Length - 1;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::UnloadSound( SPUSound& TargetSound )
    {
        TargetSound.Samples.clear();
        TargetSound.Length = 0;
    }
    
    
    // =============================================================================
    //      V32 SPU: I/O BUS CONNECTION
    // =============================================================================
    
    
    bool V32SPU::ReadPort( int32_t LocalPort, V32Word& Result )
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
            V32Word* SPURegisters = (V32Word*)(&Command);
            Result = SPURegisters[ LocalPort ];
        }
        
        // CASE 2: Read from sound-level parameters
        else if( LocalPort < (int32_t)SPU_LocalPorts::ChannelState )
        {
            V32Word* SoundRegisters = (V32Word*)PointedSound;
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
                V32Word* ChannelRegisters = (V32Word*)PointedChannel;
                int32_t ChannelPort = LocalPort - (int32_t)SPU_LocalPorts::ChannelState;
                Result = ChannelRegisters[ ChannelPort ];
            }
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32SPU::WritePort( int32_t LocalPort, V32Word Value )
    {
        // check range
        if( LocalPort > SPU_LastPort )
          return false;
        
        // redirect to the needed specific writer
        return SPUPortWriterTable[ LocalPort ]( *this, Value );
    }
    
    
    // =============================================================================
    //      V32 SPU: GENERAL OPERATION
    // =============================================================================
    
    
    void V32SPU::ChangeFrame()
    {
        // generate sound for next frame
        UpdateOutputBuffer();
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::Reset()
    {
        // reset registers
        GlobalVolume = 1.0;
        SelectedSound = -1;
        SelectedChannel = 0;
        
        // reset pointed entities
        PointedSound = &BiosSound;
        PointedChannel = &Channels[ 0 ];
        
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
        
        // reset output buffers
        memset( OutputBuffer.Samples, 0, Constants::SPUSamplesPerFrame * 4 );
        OutputBuffer.SequenceNumber = 0;
        
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
    }
    
    
    // =============================================================================
    //      V32 SPU: EXECUTION OF SPU COMMANDS
    // =============================================================================
    
    
    void V32SPU::PlayChannel( SPUChannel& TargetChannel )
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
    
    void V32SPU::PauseChannel( SPUChannel& TargetChannel )
    {
        TargetChannel.State = IOPortValues::SPUChannelState_Paused;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::StopChannel( SPUChannel& TargetChannel )
    {
        TargetChannel.State = IOPortValues::SPUChannelState_Stopped;
        
        // when stopping, rewind sound
        // (but keep sound and configuration)
        TargetChannel.Position = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::PauseAllChannels()
    {
        for( int i = 0; i < Constants::SPUSoundChannels; i++ )
          if( Channels[ i ].State == IOPortValues::SPUChannelState_Playing )
            PauseChannel( Channels[ i ] );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::ResumeAllChannels()
    {
        for( int i = 0; i < Constants::SPUSoundChannels; i++ )
          if( Channels[ i ].State == IOPortValues::SPUChannelState_Paused )
            PlayChannel( Channels[ i ] );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32SPU::StopAllChannels()
    {
        for( int i = 0; i < Constants::SPUSoundChannels; i++ )
          if( Channels[ i ].State != IOPortValues::SPUChannelState_Stopped )
            StopChannel( Channels[ i ] );
    }
    
    
    // =============================================================================
    //      V32 SPU: GENERATE SOUND OUTPUT
    // =============================================================================
    
    
    void V32SPU::UpdateOutputBuffer()
    {
        // assign the next sequence number to the buffer
        OutputBuffer.SequenceNumber++;
        
        // determine the value for each sample in the buffer
        for( int s = 0; s < Constants::SPUSamplesPerFrame; s++ )
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
            
            OutputBuffer.Samples[ s ] = ThisSample;
        }
    }
}
