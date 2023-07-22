// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/NumericFunctions.hpp"
    
    // include project headers
    #include "V32SPU.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Math
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      PORT WRITE FUNCTIONS FOR V32 SPU
    // =============================================================================
    
    
    bool WriteSPUCommand( V32SPU& SPU, V32Word Value )
    {
        // now execute the command, if valid
        switch( Value.AsInteger )
        {
            case (int32_t)IOPortValues::SPUCommand_PlaySelectedChannel:
                SPU.PlayChannel( *SPU.PointedChannel );
                break;
                
            case (int32_t)IOPortValues::SPUCommand_PauseSelectedChannel:
                SPU.PauseChannel( *SPU.PointedChannel );
                break;
                
            case (int32_t)IOPortValues::SPUCommand_StopSelectedChannel:
                SPU.StopChannel( *SPU.PointedChannel );
                break;
                
            case (int32_t)IOPortValues::SPUCommand_PauseAllChannels:
                SPU.PauseAllChannels();
                break;
                
            case (int32_t)IOPortValues::SPUCommand_ResumeAllChannels:
                SPU.ResumeAllChannels();
                break;
                
            case (int32_t)IOPortValues::SPUCommand_StopAllChannels:
                SPU.StopAllChannels();
                break;
                
            // unknown command codes are ignored, but
            // they don't trigger a port write error
            default: break;
        }
        
        // do not write the value;
        // it is useless anyway (this port is write-only)
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUGlobalVolume( V32SPU& SPU, V32Word Value )
    {
        // Float parameters are only written if they are valid
        // numeric values (otherwise the request is ignored).
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, 0, 2 );
        SPU.GlobalVolume = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSelectedSound( V32SPU& SPU, V32Word Value )
    {
        // prevent setting a non-existent sound
        if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)SPU.CartridgeSounds.size() )
          return true;
        
        // write the value
        SPU.SelectedSound = Value.AsInteger;
        
        // now update the pointed entity
        if( Value.AsInteger == -1 )
        {
            // special case for BIOS sound
            SPU.PointedSound = &SPU.BiosSound;
        }
        else
        {
            // regular cartridge sounds
            SPU.PointedSound = &SPU.CartridgeSounds[ SPU.SelectedSound ];
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSelectedChannel( V32SPU& SPU, V32Word Value )
    {
        // prevent setting a non-existent channel
        if( Value.AsInteger < 0 || Value.AsInteger >= (int32_t)Constants::SPUSoundChannels )
          return true;
        
        // write the value
        SPU.SelectedChannel = Value.AsInteger;
        
        // update pointed entity
        SPU.PointedChannel = &SPU.Channels[ SPU.SelectedChannel ];
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSoundLength( V32SPU& SPU, V32Word Value )
    {
        // reject the request: this port is read-only
        return false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSoundPlayWithLoop( V32SPU& SPU, V32Word Value )
    {
        // write the value as a boolean
        SPU.PointedSound->PlayWithLoop = (Value.AsBinary != 0? 1 : 0);
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSoundLoopStart( V32SPU& SPU, V32Word Value )
    {
        // out of range values are accepted, but clamped
        Clamp( Value.AsInteger, 0, SPU.PointedSound->Length - 1 );
        
        // enforce that LoopEnd >= LoopStart
        SPU.PointedSound->LoopStart = min( Value.AsInteger, SPU.PointedSound->LoopEnd );
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUSoundLoopEnd( V32SPU& SPU, V32Word Value )
    {
        // out of range values are accepted, but clamped
        Clamp( Value.AsInteger, 0, SPU.PointedSound->Length - 1 );
        
        // enforce that LoopEnd >= LoopStart
        SPU.PointedSound->LoopEnd = max( Value.AsInteger, SPU.PointedSound->LoopStart );
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelState( V32SPU& SPU, V32Word Value )
    {
        // reject the request: this port is read-only
        return false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelAssignedSound( V32SPU& SPU, V32Word Value )
    {
        // prevent setting a non-existent sound
        if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)SPU.CartridgeSounds.size() )
          return true;
        
        // sounds can only be assigned to a non playing channel
        if( SPU.PointedChannel->State != IOPortValues::SPUChannelState_Stopped )
          return true;
        
        // write the value
        SPU.PointedChannel->AssignedSound = Value.AsInteger;
        
        // now update the channel's current sound
        if( Value.AsInteger == -1 )
        {
            // special case for BIOS sound
            SPU.PointedChannel->CurrentSound = &SPU.BiosSound;
        }
        else
        {
            // regular cartridge sounds
            SPU.PointedChannel->CurrentSound = &SPU.CartridgeSounds[ Value.AsInteger ];
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelVolume( V32SPU& SPU, V32Word Value )
    {
        // Float parameters are only written if they are valid
        // numeric values (otherwise the request is ignored).
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, 0, 8 );
        SPU.PointedChannel->Volume = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelSpeed( V32SPU& SPU, V32Word Value )
    {
        // Float parameters are only written if they are valid
        // numeric values (otherwise the request is ignored).
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, 0, 128 );
        SPU.PointedChannel->Speed = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelLoopEnabled( V32SPU& SPU, V32Word Value )
    {
        // write the value as a boolean
        SPU.PointedChannel->LoopEnabled = (Value.AsBinary != 0? 1 : 0);
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteSPUChannelPosition( V32SPU& SPU, V32Word Value )
    {
        // out of range values are accepted, but they are clamped
        int32_t SoundLength = SPU.PointedChannel->CurrentSound->Length;
        Clamp( Value.AsInteger, 0, SoundLength - 1 );
        
        // write the value as an integer
        // (decimal part will be reset to zero)
        SPU.PointedChannel->Position = Value.AsInteger;
        return true;
    }
}
