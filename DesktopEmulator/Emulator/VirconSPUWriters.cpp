// *****************************************************************************
    // include project headers
    #include "VirconSPU.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


void WriteSPUCommand( VirconSPU& SPU, VirconWord Value )
{
    // now execute the command, if valid
    switch( Value.AsInteger )
    {
        case (int32_t)IOPortValues::SPUCommand_PlaySelectedChannel:
            SPU.PlayChannel( *SPU.PointedChannel );
            return;
            
        case (int32_t)IOPortValues::SPUCommand_PauseSelectedChannel:
            SPU.PauseChannel( *SPU.PointedChannel );
            return;
            
        case (int32_t)IOPortValues::SPUCommand_StopSelectedChannel:
            SPU.StopChannel( *SPU.PointedChannel );
            return;
            
        case (int32_t)IOPortValues::SPUCommand_PauseAllChannels:
            SPU.PauseAllChannels();
            return;
            
        case (int32_t)IOPortValues::SPUCommand_ResumeAllChannels:
            SPU.ResumeAllChannels();
            return;
            
        case (int32_t)IOPortValues::SPUCommand_StopAllChannels:
            SPU.StopAllChannels();
            return;
            
        // (unknown command codes are just ignored)
        default: return;
    }
    
    // do not write the value;
    // it is useless anyway (this port is write-only)
}

// -----------------------------------------------------------------------------

void WriteSPUGlobalVolume( VirconSPU& SPU, VirconWord Value )
{
    // Float parameters are only written if they are valid
    // numeric values (otherwise the request is ignored).
    if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
      return;
    
    // out of range values are accepted, but they are clamped
    Clamp( Value.AsFloat, 0, 2 );
    SPU.GlobalVolume = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteSPUSelectedSound( VirconSPU& SPU, VirconWord Value )
{
    // prevent setting a non-existent sound
    if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)SPU.CartridgeSounds.size() )
      return;
    
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
}

// -----------------------------------------------------------------------------

void WriteSPUSelectedChannel( VirconSPU& SPU, VirconWord Value )
{
    // prevent setting a non-existent channel
    if( Value.AsInteger < 0 || Value.AsInteger >= (int32_t)Constants::SPUSoundChannels )
      return;
    
    // write the value
    SPU.SelectedChannel = Value.AsInteger;
    
    // update pointed entity
    SPU.PointedChannel = &SPU.Channels[ SPU.SelectedChannel ];
}

// -----------------------------------------------------------------------------

void WriteSPUSoundLength( VirconSPU& SPU, VirconWord Value )
{
    // ignore the request: this port is read-only
    return;
}

// -----------------------------------------------------------------------------

void WriteSPUSoundPlayWithLoop( VirconSPU& SPU, VirconWord Value )
{
    // write the value as a boolean
    SPU.PointedSound->PlayWithLoop = (Value.AsBinary != 0? 1 : 0);
}

// -----------------------------------------------------------------------------

void WriteSPUSoundLoopStart( VirconSPU& SPU, VirconWord Value )
{
    // out of range values are accepted, but clamped
    Clamp( Value.AsInteger, 0, SPU.PointedSound->Length - 1 );
    
    // enforce that LoopEnd >= LoopStart
    SPU.PointedSound->LoopStart = min( Value.AsInteger, SPU.PointedSound->LoopEnd );
}

// -----------------------------------------------------------------------------

void WriteSPUSoundLoopEnd( VirconSPU& SPU, VirconWord Value )
{
    // out of range values are accepted, but clamped
    Clamp( Value.AsInteger, 0, SPU.PointedSound->Length - 1 );
    
    // enforce that LoopEnd >= LoopStart
    SPU.PointedSound->LoopEnd = max( Value.AsInteger, SPU.PointedSound->LoopStart );
}

// -----------------------------------------------------------------------------

void WriteSPUChannelState( VirconSPU& SPU, VirconWord Value )
{
    // ignore the request: this port is read-only
    return;
}

// -----------------------------------------------------------------------------

void WriteSPUChannelAssignedSound( VirconSPU& SPU, VirconWord Value )
{
    // prevent setting a non-existent sound
    if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)SPU.CartridgeSounds.size() )
      return;
    
    // sounds can only be assigned to a non playing channel
    if( SPU.PointedChannel->State != IOPortValues::SPUChannelState_Stopped )
      return;
    
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
}

// -----------------------------------------------------------------------------

void WriteSPUChannelVolume( VirconSPU& SPU, VirconWord Value )
{
    // Float parameters are only written if they are valid
    // numeric values (otherwise the request is ignored).
    if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
      return;
    
    // out of range values are accepted, but they are clamped
    Clamp( Value.AsFloat, 0, 8 );
    SPU.PointedChannel->Volume = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteSPUChannelSpeed( VirconSPU& SPU, VirconWord Value )
{
    // Float parameters are only written if they are valid
    // numeric values (otherwise the request is ignored).
    if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
      return;
    
    // out of range values are accepted, but they are clamped
    Clamp( Value.AsFloat, 0, 128 );
    SPU.PointedChannel->Speed = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteSPUChannelLoopEnabled( VirconSPU& SPU, VirconWord Value )
{
    // write the value as a boolean
    SPU.PointedChannel->LoopEnabled = (Value.AsBinary != 0? 1 : 0);
}

// -----------------------------------------------------------------------------

void WriteSPUChannelPosition( VirconSPU& SPU, VirconWord Value )
{
    // out of range values are accepted, but they are clamped
    int32_t SoundLength = SPU.PointedChannel->CurrentSound->Length;
    Clamp( Value.AsInteger, 0, SoundLength - 1 );
    
    // write the value as an integer
    // (decimal part will be reset to zero)
    SPU.PointedChannel->Position = Value.AsInteger;
}
