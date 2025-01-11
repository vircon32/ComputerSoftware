// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "Savestates.hpp"
    #include "VideoOutput.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <memory>             // [ C++ STL ] Dynamic memory
    #include <string.h>           // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      SERIALIZATION (SAVE CONSOLE STATE TO BUFFER)
// =============================================================================


void SaveCPUState( CPUState& State )
{
    // all fields should be adjacent in memory
    // so read registers and flags together
    memcpy( &State, Console.CPU.Registers, sizeof(CPUState) );
}

// -----------------------------------------------------------------------------

void SaveGPUState( GPUState& State )
{
    V32GPU& GPU = Console.GPU;
    
    // read all registers as adjacent
    memcpy( State.Registers, &GPU.Command, sizeof(State.Registers) );
    
    // copy the BIOS texture
    memcpy( &State.BiosTexture, &GPU.BiosTexture, sizeof(GPUTexture) );
    
    // copy only the needed cartridge textures
    unsigned TexturesSize = sizeof(GPUTexture) * GPU.LoadedCartridgeTextures;
    memcpy( State.CartridgeTextures, &GPU.CartridgeTextures[ 0 ], TexturesSize );
}

// -----------------------------------------------------------------------------

void SaveSPUState( SPUState& State )
{
    V32SPU& SPU = Console.SPU;
    
    // read all registers as adjacent
    memcpy( State.Registers, &SPU.Command, sizeof(State.Registers) );
    
    // read all channels as adjacent
    memcpy( State.Channels, &SPU.Channels, sizeof(State.Channels) );
    
    // copy the BIOS sound
    memcpy( &State.BiosSound, &SPU.BiosSound, sizeof(SPUSoundState) );
    
    // copy only the needed cartridge sounds
    // (size will stay the same, but speed will increase)
    unsigned CartridgeSounds = SPU.LoadedCartridgeSounds;
    
    // do not read data for all sounds as a block!!
    // we don't want to copy the sample vector in each sound
    for( unsigned SoundID = 0; SoundID < CartridgeSounds; SoundID++ )
      memcpy( &State.CartridgeSounds[ SoundID ], &SPU.CartridgeSounds[ SoundID ], sizeof(SPUSoundState) );
}

// -----------------------------------------------------------------------------

void SaveGamepadControllerState( GamepadControllerState& State )
{
    State.SelectedGamepad = Console.GamepadController.SelectedGamepad;
    
    // read all gamepad states as adjacent
    memcpy( State.GamepadStates, Console.GamepadController.RealTimeGamepadStates, sizeof(State.GamepadStates) );
}

// -----------------------------------------------------------------------------

void SaveOtherConsoleState( OtherConsoleState& State )
{
    // save state for minor chips
    memcpy( State.TimerRegisters, &Console.Timer.CurrentDate, sizeof(State.TimerRegisters) );
    State.RNGCurrentValue = Console.RNG.CurrentValue;
    
    // save the full RAM
    memcpy( State.RAM, &Console.RAM.Memory[ 0 ], sizeof(State.RAM) );
}

// -----------------------------------------------------------------------------

void SaveGameInfo( ROMInfo& Info )
{
    // ensure title does not exceed 64 bytes and
    // that its unused characters are all null
    memset( Info.Title, 0, sizeof(Info.Title) );
    strncpy( Info.Title, Console.CartridgeController.CartridgeTitle.c_str(), sizeof(Info.Title) - 1 );
    
    Info.Version = Console.CartridgeController.CartridgeVersion;
    Info.Revision = Console.CartridgeController.CartridgeRevision;
    Info.ProgramROMSize = Console.CartridgeController.MemorySize;
    Info.NumberOfTextures = Console.CartridgeController.NumberOfTextures;
    Info.NumberOfSounds = Console.CartridgeController.NumberOfSounds;
}

// -----------------------------------------------------------------------------

void SaveBiosInfo( ROMInfo& Info )
{
    // ensure title does not exceed 64 bytes and
    // that its unused characters are all null
    memset( Info.Title, 0, sizeof(Info.Title) );
    strncpy( Info.Title, Console.BiosTitle.c_str(), sizeof(Info.Title) - 1 );
    
    Info.Version = Console.BiosVersion;
    Info.Revision = Console.BiosRevision;
    Info.ProgramROMSize = Console.BiosProgramROM.MemorySize;
    Info.NumberOfTextures = 1;
    Info.NumberOfSounds = 1;
}

// -----------------------------------------------------------------------------

void SaveState( ConsoleState* State )
{
    // save info to identify the game and BIOS
    SaveGameInfo( State->Game );
    SaveBiosInfo( State->Bios );
    
    // save console state
    SaveCPUState( State->CPU );
    SaveGPUState( State->GPU );
    SaveSPUState( State->SPU );
    SaveGamepadControllerState( State->GamepadController );
    SaveOtherConsoleState( State->Others );
}


// =============================================================================
//      DESERIALIZATION (LOAD CONSOLE STATE FROM BUFFER)
// =============================================================================


void LoadCPUState( const CPUState& State )
{
    // all fields should be adjacent in memory
    // so write registers and flags together
    memcpy( Console.CPU.Registers, &State, sizeof(CPUState) );
}

// -----------------------------------------------------------------------------

void LoadGPUState( const GPUState& State )
{
    V32GPU& GPU = Console.GPU;
    
    // write all registers as adjacent
    memcpy( &GPU.Command, State.Registers, sizeof(State.Registers) );
    
    // copy the BIOS texture
    memcpy( &GPU.BiosTexture, &State.BiosTexture, sizeof(GPUTexture) );
    
    // copy only the needed cartridge textures
    unsigned TexturesSize = sizeof(GPUTexture) * GPU.LoadedCartridgeTextures;
    memcpy( &GPU.CartridgeTextures[ 0 ], State.CartridgeTextures, TexturesSize );
    
    // update GPU pointers for the loaded selections
    if( GPU.SelectedTexture == -1 )
      GPU.PointedTexture = &GPU.BiosTexture;
    else
      GPU.PointedTexture = &GPU.CartridgeTextures[ GPU.SelectedTexture ];
    
    GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
    
    // reset any previous OpenGL errors
    while( glGetError() != GL_NO_ERROR )
    {
        // (empty block instead of ";" to avoid warnings)
    }
    
    // make the needed updates in video output
    Video.SelectTexture( GPU.SelectedTexture );
    Video.SetMultiplyColor( GPU.MultiplyColor );
    Video.SetBlendingMode( (IOPortValues)GPU.ActiveBlending );
    
    // check for success
    if( glGetError() != GL_NO_ERROR )
      THROW( "There was an OpenGL error" );
}

// -----------------------------------------------------------------------------

void LoadSPUState( const SPUState& State )
{
    V32SPU& SPU = Console.SPU;
    
    // write all registers as adjacent
    memcpy( &SPU.Command, State.Registers, sizeof(State.Registers) );
    
    // write all channels as adjacent
    memcpy( &SPU.Channels, State.Channels, sizeof(State.Channels) );
    
    // copy the BIOS sound
    memcpy( (void*)&SPU.BiosSound, &State.BiosSound, sizeof(SPUSoundState) );
    
    // copy only the needed cartridge sounds
    // (size will stay the same, but speed will increase)
    unsigned CartridgeSounds = SPU.LoadedCartridgeSounds;
    
    // do not load data for all sounds as a block!!
    // we must not overwrite the sample vector in each sound
    for( unsigned SoundID = 0; SoundID < CartridgeSounds; SoundID++ )
      memcpy( (void*)&SPU.CartridgeSounds[ SoundID ], &State.CartridgeSounds[ SoundID ], sizeof(SPUSoundState) );
    
    // make the needed updates in audio objects
    V32Word WordValue;
    
    WordValue.AsInteger = SPU.SelectedSound;
    SPU.WritePort( (int32_t)SPU_LocalPorts::SelectedSound, WordValue );
    
    WordValue.AsInteger = SPU.SelectedChannel;
    SPU.WritePort( (int32_t)SPU_LocalPorts::SelectedChannel, WordValue );
    
    // update SPU pointers for the loaded selections
    if( SPU.SelectedSound == -1 )
      SPU.PointedSound = &SPU.BiosSound;
    else
      SPU.PointedSound = &SPU.CartridgeSounds[ SPU.SelectedSound ];
    
    SPU.PointedChannel = &SPU.Channels[ SPU.SelectedChannel ];
}

// -----------------------------------------------------------------------------

void LoadGamepadControllerState( const GamepadControllerState& State )
{
    // write the single exposed register
    Console.GamepadController.SelectedGamepad = State.SelectedGamepad;
    
    // write all gamepad states as adjacent
    memcpy( Console.GamepadController.RealTimeGamepadStates, State.GamepadStates, sizeof(State.GamepadStates) );
}

// -----------------------------------------------------------------------------

void LoadOtherConsoleState( const OtherConsoleState& State )
{
    // load state for minor chips
    memcpy( &Console.Timer.CurrentDate, State.TimerRegisters, sizeof(State.TimerRegisters) );
    Console.RNG.CurrentValue = State.RNGCurrentValue;
    
    // load the full RAM
    memcpy( &Console.RAM.Memory[ 0 ], State.RAM, sizeof(State.RAM) );
}

// -----------------------------------------------------------------------------

void LoadState( const ConsoleState* State )
{
    // try to identify the game and BIOS and see if they
    // match current ones, to avoid loading incompatible states
    ROMInfo CurrentGame, CurrentBios;
    SaveGameInfo( CurrentGame );
    SaveBiosInfo( CurrentBios );
    
    if( memcmp( &State->Game, &CurrentGame, sizeof(ROMInfo) ) )
      THROW( "Current cartridge is not the same one that was saved" );
    
    if( memcmp( &State->Bios, &CurrentBios, sizeof(ROMInfo) ) )
      THROW( "Current BIOS is not the same one that was used when saving" );
    
    // load console state
    LoadCPUState( State->CPU );
    LoadSPUState( State->SPU );
    LoadGPUState( State->GPU );
    LoadGamepadControllerState( State->GamepadController );
    LoadOtherConsoleState( State->Others );
}


// =============================================================================
//      RLE BUFFER COMPRESSION
// -----------------------------------------------------------------------------
//      The full size of a Vircon32 savestate is 16+ MB, however most of the
//      RAM will typically be zeroes so we will store savestates on disk
//      using a very simple RLE compression that should reduce size a lot
// =============================================================================


unsigned GetSavestateSize()
{
    // savestates may be a different size for each
    // game, that is fine by libretro as long as
    // that size is always the same for each game
    unsigned UnusedCartridgeTextures = V32::Constants::GPUMaximumCartridgeTextures;
    
    if( Console.HasCartridge() )
      UnusedCartridgeTextures -= Console.GPU.LoadedCartridgeTextures;
    
    return sizeof( ConsoleState ) - UnusedCartridgeTextures * sizeof( V32::GPUTexture );
}

// -----------------------------------------------------------------------------

void SaveBufferToRLEFile( ofstream& OutputFile, const void* Buffer )
{
    LOG( "Compressing state file" );
    
    // determine buffer size
    unsigned SavestateSize = GetSavestateSize();
    unsigned CompressedSize = 0;
    unsigned SavedSize = 0;
    
    uint8_t QuantityByte = 0;
    uint8_t* CurrentByteRead = (uint8_t*)Buffer;
    uint8_t CurrentValue = *CurrentByteRead;
    
    while( CompressedSize < SavestateSize )
    {
        bool ValueRepeats = (*CurrentByteRead == CurrentValue);
        
        if( ValueRepeats )
        {
            // add 1 to the current pair
            QuantityByte++;
        }
        
        if( !ValueRepeats || QuantityByte == 255 )
        {
            // output previous pair
            OutputFile << QuantityByte;
            OutputFile << CurrentValue;
            SavedSize += QuantityByte;
            
            // restart for next pair
            QuantityByte = (QuantityByte == 255? 0 : 1);
            CurrentValue = *CurrentByteRead;
        }
        
        CurrentByteRead++;
        CompressedSize++;
    }
    
    // output last pair if needed
    if( SavedSize < CompressedSize )
    {
        OutputFile << QuantityByte;
        OutputFile << CurrentValue;
    }
}

// -----------------------------------------------------------------------------

void LoadBufferFromRLEFile( ifstream& InputFile, void* Buffer )
{
    LOG( "Decompressing state file" );
    
    unsigned DecompressedSize = 0;
    uint8_t* CurrentByteSaved = (uint8_t*)Buffer;
    uint8_t QuantityByte;
    uint8_t CurrentValue;
    
    while( InputFile.peek() != EOF )
    {
        // read the next quantity-value pair of bytes
        InputFile.read( (char*)&QuantityByte, 1 );
        
        if( InputFile.peek() == EOF )
          THROW( "Compressed file is corrupt" );
        
        InputFile.read( (char*)&CurrentValue, 1 );
        
        // write the string of values to the buffer
        memset( CurrentByteSaved, CurrentValue, QuantityByte );
        
        CurrentByteSaved += QuantityByte;
        DecompressedSize += QuantityByte;
        
        // we should never exceed the maximum savestate size
        // (or else we will write into unknown memory areas)
        if( DecompressedSize >= sizeof(ConsoleState) )
          THROW( "Decompressed file size is too large" );
    }
    
    // determine the actual savestate size for this game
    unsigned SavestateSize = GetSavestateSize();
    
    // verify final buffer size
    if( DecompressedSize != SavestateSize )
      THROW( "Decompressed file size is not correct" );
}


// =============================================================================
//      LOAD/SAVE CONSOLE STATE TO A FILE
// =============================================================================


void SaveState( const string& FileName )
{
    LOG( "Saving state in slot " + to_string(SavestatesSlot) );
    
    // save the state from console into the buffer
    unique_ptr< ConsoleState > StateBuffer( new ConsoleState );
    SaveState( StateBuffer.get() );
    
    // open the file
    ofstream OutputFile;
    OpenOutputFile( OutputFile, FileName, ios_base::out | ios_base::binary );
    
    if( !OutputFile.good() )
      THROW( "Cannot open output file" );
    
    // save and compress the console state into that file
    SaveBufferToRLEFile( OutputFile, StateBuffer.get() );
    OutputFile.close();
}

// -----------------------------------------------------------------------------

void LoadState( const string& FileName )
{
    LOG( "Loading state from slot " + to_string(SavestatesSlot) );
    
    // open the file
    ifstream InputFile;
    OpenInputFile( InputFile, FileName, ios_base::in | ios_base::binary );
    
    if( !InputFile.good() )
      THROW( "Cannot open input file" );
    
    // load and decompressed the console state from that file
    unique_ptr< ConsoleState > StateBuffer( new ConsoleState );
    LoadBufferFromRLEFile( InputFile, StateBuffer.get() );
    InputFile.close();
    
    // load the state from the buffer into the console
    LoadState( StateBuffer.get() );
}
