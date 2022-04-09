/* *****************************************************************************
*  Vircon32 standard library: "audio.h"           File version: 2022/02/25     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all definitions and functions needed by the programs to  *
*  interact with Vircon SPU and control audio in general. This includes all    *
*  general SPU configuration, as well as specific functions for sound buffers  *
*  and audio channels.                                                         *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef AUDIO_H
    #define AUDIO_H
// *****************************************************************************


// =============================================================================
//   GENERAL VIRCON DEFINITIONS
// =============================================================================


// properties of the audio system
#define sound_channels 16

// states of a sound channel
#define channel_stopped  0x40
#define channel_paused   0x41
#define channel_playing  0x42


// =============================================================================
//   FUNCTIONS FOR SPU SELECTED ELEMENTS
// =============================================================================


void select_sound( int sound_id )
{
    asm
    {
        "mov R0, {sound_id}"
        "out SPU_SelectedSound, R0"
    }
}

// -----------------------------------------------------------------------------

int get_selected_sound()
{
    asm
    {
        "in R0, SPU_SelectedSound"
    }
}

// -----------------------------------------------------------------------------

void select_channel( int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
    }
}

// -----------------------------------------------------------------------------

int get_selected_channel()
{
    asm
    {
        "in R0, SPU_SelectedChannel"
    }
}


// =============================================================================
//   CONFIGURATION OF SPU SOUNDS
// =============================================================================


// applies to the currently selected sound
void set_sound_loop( bool enabled )
{
    asm
    {
        "mov R0, {enabled}"
        "out SPU_SoundPlayWithLoop, R0"
    }
}

// -----------------------------------------------------------------------------

// position is given from sound start in number of samples
void set_sound_loop_start( int position )
{
    asm
    {
        "mov R0, {position}"
        "out SPU_SoundLoopStart, R0"
    }
}

// -----------------------------------------------------------------------------

// position is given from sound start in number of samples
void set_sound_loop_end( int position )
{
    asm
    {
        "mov R0, {position}"
        "out SPU_SoundLoopEnd, R0"
    }
}


// =============================================================================
//   CONFIGURATION OF SPU CHANNELS
// =============================================================================


// applies to the currently selected channel
void set_channel_volume( float volume )
{
    asm
    {
        "mov R0, {volume}"
        "out SPU_ChannelVolume, R0"
    }
}

// -----------------------------------------------------------------------------

// applies to the currently selected channel
void set_channel_speed( float speed )
{
    asm
    {
        "mov R0, {speed}"
        "out SPU_ChannelSpeed, R0"
    }
}

// -----------------------------------------------------------------------------

// applies to the currently selected channel
void set_channel_loop( bool enabled )
{
    asm
    {
        "mov R0, {enabled}"
        "out SPU_ChannelLoopEnabled, R0"
    }
}

// -----------------------------------------------------------------------------

void assign_channel_sound( int channel_id, int sound_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "mov R0, {sound_id}"
        "out SPU_ChannelAssignedSound, R0"
    }
}

// -----------------------------------------------------------------------------

int get_channel_state( int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "in R0, SPU_ChannelState"
    }    
}


// =============================================================================
//   SETTING GLOBAL SPU PARAMETERS
// =============================================================================


// range is 0 to 1
void set_global_volume( float volume )
{
    asm
    {
        "mov R0, {volume}"
        "out SPU_GlobalVolume, R0"
    }
}

// -----------------------------------------------------------------------------

float get_global_volume()
{
    asm
    {
        "in R0, SPU_GlobalVolume"
    }    
}


// =============================================================================
//   REQUESTING SPU COMMANDS (SINGLE CHANNEL)
// =============================================================================


void play_channel( int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "out SPU_Command, SPUCommand_PlaySelectedChannel"
    }
}

// -----------------------------------------------------------------------------

void pause_channel( int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "out SPU_Command, SPUCommand_PauseSelectedChannel"
    }
}

// -----------------------------------------------------------------------------

void stop_channel( int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "out SPU_Command, SPUCommand_StopSelectedChannel"
    }
}


// =============================================================================
//   REQUESTING SPU COMMANDS (ALL CHANNELS)
// =============================================================================


void pause_all_channels()
{
    asm
    {
        "out SPU_Command, SPUCommand_PauseAllChannels"
    }
}

// -----------------------------------------------------------------------------

void stop_all_channels()
{
    asm
    {
        "out SPU_Command, SPUCommand_StopAllChannels"
    }
}

// -----------------------------------------------------------------------------

void resume_all_channels()
{
    asm
    {
        "out SPU_Command, SPUCommand_ResumeAllChannels"
    }
}


// =============================================================================
//   PRACTICAL SOUND PLAY FUNCTIONS
// =============================================================================


void play_sound_in_channel( int sound_id, int channel_id )
{
    asm
    {
        "mov R0, {channel_id}"
        "out SPU_SelectedChannel, R0"
        "mov R0, {sound_id}"
        "out SPU_ChannelAssignedSound, R0"
        "out SPU_Command, SPUCommand_PlaySelectedChannel"
    }
}

// -----------------------------------------------------------------------------


// this will automatically find a free channel to play,
// and return that channel ID; if there is no free
// channel, the sound will not play and the function
// will return -1 to signal it
int play_sound( int sound_id )
{
    asm
    {
        // initialize
        "push R1"
        "mov R0, 0"
        
        // loop the 16 channels
        "__play_sound_begin_loop:"
        "out SPU_SelectedChannel, R0"
        "in R1, SPU_ChannelState"
        "ieq R1, 0x40"
        "jt R1, __play_sound_channel_found"
        "iadd R0, 1"
        "mov R1, R0"
        "ige R1, 16"
        "jf R1, __play_sound_begin_loop"
        
        // no free channels
        "mov R0, -1"
        "jmp __play_sound_exit"
        
        // free channel found
        "__play_sound_channel_found:"
        "mov R1, {sound_id}"
        "out SPU_ChannelAssignedSound, R1"
        "out SPU_Command, SPUCommand_PlaySelectedChannel"
        
        // exit and return value in R0
        "__play_sound_exit:"
        "pop R1"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
