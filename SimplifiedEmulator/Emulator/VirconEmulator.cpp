// *****************************************************************************
    // include common Vircon headers
    #include "../VirconDefinitions/VirconDefinitions.hpp"
    #include "../VirconDefinitions/VirconROMFormat.hpp"
    
    // include project headers
    #include "VirconEmulator.hpp"
    #include "OpenGL2DContext.hpp"
    #include "AuxFunctions.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      VIRCON EMULATOR: INSTANCE HANDLING
// =============================================================================


VirconEmulator::VirconEmulator()
{
    // connect memory bus master
    CPU.MemoryBus = &MemoryBus;
    MemoryBus.Master = &CPU;
    
    // connect memory bus slaves
    MemoryBus.Slaves[ 0 ] = &RAM;
    MemoryBus.Slaves[ 1 ] = &BiosProgramROM;
    MemoryBus.Slaves[ 2 ] = &CartridgeController;
    MemoryBus.Slaves[ 3 ] = &MemoryCardController;
    
    // connect control bus master
    CPU.ControlBus = &ControlBus;
    ControlBus.Master = &CPU;
    
    // connect control bus slaves
    ControlBus.Slaves[ 0 ] = &Timer;
    ControlBus.Slaves[ 1 ] = &RNG;
    ControlBus.Slaves[ 2 ] = &GPU;
    ControlBus.Slaves[ 3 ] = &SPU;
    ControlBus.Slaves[ 4 ] = &GamepadController;
    ControlBus.Slaves[ 5 ] = &CartridgeController;
    ControlBus.Slaves[ 6 ] = &MemoryCardController;
    ControlBus.Slaves[ 7 ] = &NullController;
    
    // connect main RAM
    RAM.Connect( Constants::RAMSize );
    
    // connect 2 gamepads
    GamepadController.ProcessConnectionChange( 0, true );
    GamepadController.ProcessConnectionChange( 1, true );
    
    // set initial state
    PowerIsOn = false;
    Paused = false;
    
    // do NOT reset until power on
}

// -----------------------------------------------------------------------------

VirconEmulator::~VirconEmulator()
{
    // (do nothing, for now)
}

       
// =============================================================================
//      VIRCON EMULATOR: BIOS MANAGEMENT
// =============================================================================


void VirconEmulator::LoadBios( const std::string& FilePath )
{
    // open bios file
    cout << "Loading bios" << endl;

    ifstream InputFile;
    InputFile.open( FilePath, ios_base::binary | ios_base::ate );
    
    if( InputFile.fail() )
      throw runtime_error( "Cannot open BIOS file" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Load global information
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = InputFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "Incorrect V32 file format (file size must be a multiple of 4)" );
    
    // ensure that we can at least load the file header
    if( FileBytes < sizeof(ROMFileHeader) )
      throw runtime_error( "Incorrect V32 file format (file is too small)" );
    
    // now we can safely read the global header
    InputFile.seekg( 0, ios_base::beg );
    ROMFileHeader ROMHeader;
    InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileHeader) );
    
    // check if the ROM is actually a cartridge
    if( CheckSignature( ROMHeader.Signature, Signatures::CartridgeFile ) )
      throw runtime_error( "Input V32 ROM cannot be loaded as a BIOS (is it a cartridge instead)" );
    
    // now check the actual BIOS signature
    if( !CheckSignature( ROMHeader.Signature, Signatures::BiosFile ) )
      throw runtime_error( "Incorrect V32 file format (file does not have a valid signature)" );
    
    // check current Vircon version
    if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
    ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
      throw runtime_error( "This BIOS was made for a more recent version of Vircon. Please use an updated emulator" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Check the declared rom contents
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // ensure that there is exactly 1 texture
    if( ROMHeader.NumberOfTextures != 1 )
      throw runtime_error( "A BIOS video rom should have exactly 1 texture" );
    
    // ensure that there is exactly 1 sound
    if( ROMHeader.NumberOfSounds != 1 )
      throw runtime_error( "A BIOS audio rom should have exactly 1 sound" );
    
    // check for correct program rom location
    if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileHeader) )
      throw runtime_error( "Incorrect V32 file format (program ROM is not located after file header)" );
    
    // check for correct video rom location
    uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
    
    if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
      throw runtime_error( "Incorrect V32 file format (video ROM is not located after program ROM)" );
    
    // check for correct audio rom location
    uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
    
    if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
      throw runtime_error( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
    
    // check for correct file size
    uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
    
    if( FileBytes != SizeAfterAudioROM )
      throw runtime_error( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Load program rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // load a binary file header
    BinaryFileHeader BinaryHeader;
    InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileHeader) );
    
    // check signature for embedded binary
    if( !CheckSignature( BinaryHeader.Signature, Signatures::BinaryFile ) )
      throw runtime_error( "BIOS binary does not have a valid signature" );
    
    // checking program rom size limitations
    if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumBiosProgramROM ) )
      throw runtime_error( "BIOS binary does not have a correct size (from 1 word up to 1M words)" );
    
    // load the binary contents
    vector< VirconWord > LoadedBinary;
    LoadedBinary.resize( BinaryHeader.NumberOfWords );
    InputFile.read( (char*)(&LoadedBinary[0]), BinaryHeader.NumberOfWords*4 );
    BiosProgramROM.Connect( &LoadedBinary[0], BinaryHeader.NumberOfWords );
    
    // discard the temporary buffer
    LoadedBinary.clear();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Load video rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // load a texture file signature
    TextureFileHeader TextureHeader;
    InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileHeader) );
    
    // check signature for embedded texture
    if( !CheckSignature( TextureHeader.Signature, Signatures::TextureFile ) )
      throw runtime_error( "BIOS texture does not have a valid signature" );
    
    // report texture size
    // check texture size limitations
    if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
    ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
      throw runtime_error( "BIOS texture does not have correct dimensions (from 1x1 up to 1024x1024 pixels)" );
    
    // load the texture pixels
    uint32_t TexturePixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
    vector< VirconWord > LoadedTexture;
    LoadedTexture.resize( TexturePixels );
    InputFile.read( (char*)(&LoadedTexture[0]), TexturePixels*4 );
    GPU.LoadTexture( GPU.BiosTexture, &LoadedTexture[0], TextureHeader.TextureWidth, TextureHeader.TextureHeight );
    
    // discard the temporary buffer
    LoadedTexture.clear();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: Load audio rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // load a sound file signature
    SoundFileHeader SoundHeader;
    InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileHeader) );
    
    // check signature for embedded sound
    if( !CheckSignature( SoundHeader.Signature, Signatures::SoundFile ) )
      throw runtime_error( "BIOS sound does not have a valid signature" );
    
    // check sound length limitations
    if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumBiosSamples ) )
      throw runtime_error( "BIOS sound does not have a correct length (from 1 up to 1M samples)" );
    
    // load the sound samples
    vector< SPUSample > LoadedSound;
    LoadedSound.resize( SoundHeader.SoundSamples );
    InputFile.read( (char*)(&LoadedSound[0]), SoundHeader.SoundSamples*4 );
    SPU.LoadSound( SPU.BiosSound, &LoadedSound[0], SoundHeader.SoundSamples );
    
    // discard the temporary buffer
    LoadedSound.clear();
    
    // close the file
    InputFile.close();
}


// =============================================================================
//      VIRCON EMULATOR: CARTRIDGE MANAGEMENT
// =============================================================================


void VirconEmulator::LoadCartridge( const std::string& FilePath )
{
    cout << "Loading cartridge" << endl;

    // unload any previous cartridge
    UnloadCartridge();
    
    // open cartridge file
    ifstream InputFile;
    InputFile.open( FilePath, ios_base::binary | ios_base::ate );
    
    if( InputFile.fail() )
      throw runtime_error( "Cannot open cartridge file" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Load global information
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = InputFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "Incorrect V32 file format (file size must be a multiple of 4)" );
    
    // ensure that we can at least load the file header
    if( FileBytes < sizeof(ROMFileHeader) )
      throw runtime_error( "Incorrect V32 file format (file is too small)" );
    
    // now we can safely read the global header
    InputFile.seekg( 0, ios_base::beg );
    ROMFileHeader ROMHeader;
    InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileHeader) );
    
    // check if the ROM is actually a BIOS
    if( CheckSignature( ROMHeader.Signature, Signatures::BiosFile ) )
      throw runtime_error( "Input V32 ROM cannot be loaded as a cartridge (is it a BIOS instead)" );
    
    // now check the actual cartridge signature
    if( !CheckSignature( ROMHeader.Signature, Signatures::CartridgeFile ) )
      throw runtime_error( "Incorrect V32 file format (file does not have a valid signature)" );
    
    // check current Vircon version
    if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
    ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
      throw runtime_error( "This cartridge was made for a more recent version of Vircon. Please use an updated emulator" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Check the declared rom contents
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // check that there are not too many textures
    if( ROMHeader.NumberOfTextures > (uint32_t)Constants::GPUMaximumCartridgeTextures )
      throw runtime_error( "Video ROM contains too many textures (Vircon GPU only allows up to 256)" );
    
    // check that there are not too many sounds
    if( ROMHeader.NumberOfSounds > (uint32_t)Constants::SPUMaximumCartridgeSounds )
      throw runtime_error( "Audio ROM contains too many sounds (Vircon SPU only allows up to 1024)" );
    
    // check for correct program rom location
    if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileHeader) )
      throw runtime_error( "Incorrect V32 file format (program ROM is not located after file header)" );
    
    // check for correct video rom location
    uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
    
    if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
      throw runtime_error( "Incorrect V32 file format (video ROM is not located after program ROM)" );
    
    // check for correct audio rom location
    uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
    
    if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
      throw runtime_error( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
    
    // check for correct file size
    uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
    
    if( FileBytes != SizeAfterAudioROM )
      throw runtime_error( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Load program rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // load a binary file signature
    BinaryFileHeader BinaryHeader;
    InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileHeader) );
    
    // check signature for embedded binary
    if( !CheckSignature( BinaryHeader.Signature, Signatures::BinaryFile ) )
      throw runtime_error( "Cartridge binary does not have a valid signature" );
    
    // check program rom size limitations
    if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumCartridgeProgramROM ) )
      throw runtime_error( "Cartridge program ROM does not have a correct size (from 1 word up to 128M words)" );
    
    // load the binary contents
    vector< VirconWord > LoadedBinary;
    LoadedBinary.resize( BinaryHeader.NumberOfWords );
    InputFile.read( (char*)(&LoadedBinary[0]), BinaryHeader.NumberOfWords*4 );
    CartridgeController.Connect( &LoadedBinary[0], BinaryHeader.NumberOfWords );
    
    // discard the temporary buffer
    LoadedBinary.clear();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Load video rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
    // load all textures in sequence
    for( unsigned i = 0; i < ROMHeader.NumberOfTextures; i++ )
    {
        // load a texture file signature
        TextureFileHeader TextureHeader;
        InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileHeader) );
        
        // check signature for embedded texture
        if( !CheckSignature( TextureHeader.Signature, Signatures::TextureFile ) )
          throw runtime_error( "Cartridge texture does not have a valid signature" );
        
        // check texture size limitations
        if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
        ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
          throw runtime_error( "Cartridge texture does not have correct dimensions (1x1 up to 1024x1024 pixels)" );
        
        // load the texture pixels
        uint32_t TexturePixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
        vector< VirconWord > LoadedTexture;
        LoadedTexture.resize( TexturePixels );
        InputFile.read( (char*)(&LoadedTexture[0]), TexturePixels*4 );
        
        // create a new GPU texture and load data into it
        GPU.CartridgeTextures.emplace_back();
        GPU.LoadTexture( GPU.CartridgeTextures.back(), &LoadedTexture[0],
                         TextureHeader.TextureWidth, TextureHeader.TextureHeight );
        
        // discard the temporary buffer
        LoadedTexture.clear();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: Load audio rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
    // keep count of the total sound samples
    uint32_t TotalSPUSamples = 0;
    
    // load all sounds in sequence
    for( unsigned i = 0; i < ROMHeader.NumberOfSounds; i++ )
    {
        // load a sound file signature
        SoundFileHeader SoundHeader;
        InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileHeader) );
        
        // check signature for embedded sound
        if( !CheckSignature( SoundHeader.Signature, Signatures::SoundFile ) )
          throw runtime_error( "Cartridge sound does not have a valid signature" );
        
        // check length limitations for this sound
        if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumCartridgeSamples ) )
          throw runtime_error( "Cartridge sound does not have correct length (1 up to 256M samples)" );
        
        // check length limitations for the whole SPU
        TotalSPUSamples += SoundHeader.SoundSamples;
        
        if( TotalSPUSamples > (uint32_t)Constants::SPUMaximumCartridgeSamples )
          throw runtime_error( "Cartridge sounds contain too many total samples (Vircon SPU only allows up to 256M total samples)" );
        
        // load the sound samples
        vector< SPUSample > LoadedSound;
        LoadedSound.resize( SoundHeader.SoundSamples );
        InputFile.read( (char*)(&LoadedSound[0]), SoundHeader.SoundSamples*4 );
        
        // create a new SPU sound and load data into it
        SPU.CartridgeSounds.emplace_back();
        SPU.LoadSound( SPU.CartridgeSounds.back(), &LoadedSound[0], SoundHeader.SoundSamples );
        
        // discard the temporary buffer
        LoadedSound.clear();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: General Vircon setup
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // finally, close input file
    InputFile.close();
}

// -----------------------------------------------------------------------------

void VirconEmulator::UnloadCartridge()
{
    // do nothing if a cartridge is not loaded
    if( !HasCartridge() ) return;
    
    cout << "Unloading cartridge" << endl;
    
    // release cartridge program ROM
    CartridgeController.Disconnect();
    
    // tell GPU to release all cartridge textures
    for( GPUTexture& T: GPU.CartridgeTextures )
      GPU.UnloadTexture( T );
    
    GPU.CartridgeTextures.clear();
    
    // tell SPU to release all cartridge sounds
    for( SPUSound& S: SPU.CartridgeSounds )
      SPU.UnloadSound( S );
    
    SPU.CartridgeSounds.clear();
}


// =============================================================================
//      VIRCON EMULATOR: MEMORY CARD MANAGEMENT
// =============================================================================


void VirconEmulator::CreateMemoryCard( const std::string& FilePath )
{
    cout << "Creating memory card" << endl;
    MemoryCardController.CreateNewFile( FilePath );
}

// -----------------------------------------------------------------------------

void VirconEmulator::LoadMemoryCard( const std::string& FilePath )
{
    cout << "Loading memory card" << endl;
    
    // unload any previous card
    UnloadMemoryCard();
    
    // load the card into memory
    Vircon.MemoryCardController.LoadContents( FilePath );
}

// -----------------------------------------------------------------------------

void VirconEmulator::UnloadMemoryCard()
{
    // do nothing if a card is not loaded
    if( !HasMemoryCard() ) return;
    
    cout << "Unloading memory card" << endl;
    
    // remove the card memory
    Vircon.MemoryCardController.Disconnect();
}


// =============================================================================
//      VIRCON EMULATOR: GENERAL OPERATION
// =============================================================================


void VirconEmulator::Initialize()
{
    // initialize audio playback
    SPU.InitializeAudio();
}

// -----------------------------------------------------------------------------

void VirconEmulator::Terminate()
{
    // terminate audio playback
    SPU.TerminateAudio();
    
    // release all connected media
    UnloadCartridge();
}

// -----------------------------------------------------------------------------

void VirconEmulator::RunNextFrame()
{
    // do nothing when not applicable
    if( !PowerIsOn || Paused )
      return;
    
    // STEP 1: Begin a new frame by sending
    // a frame change message to components
    Timer.ChangeFrame();
    CPU.ChangeFrame();
    GPU.ChangeFrame();
    SPU.ChangeFrame();
    MemoryCardController.ChangeFrame();
    GamepadController.ChangeFrame();
    
    // STEP 2: Run a frame's worth of cycles
    for( int i = 0; i < Constants::CyclesPerFrame; i++ )
    {
        // only these components need to
        // be notified of each CPU cycle
        Timer.RunNextCycle();
        CPU.RunNextCycle();
        
        // end loop early when CPU is set to wait
        if( CPU.Waiting || CPU.Halted )
          break;
    }
    
    // STEP 3: after running, ensure that all GPU
    // commands run in the current frame are drawn
    glFlush();
}

// -----------------------------------------------------------------------------

void VirconEmulator::Reset()
{
    // first: transmit the message to all components that need it
    Timer.Reset();
    RNG.Reset();
    CPU.Reset();
    GPU.Reset();
    SPU.Reset();
    GamepadController.Reset();
    
    // now reset the emulator itself
    RAM.ClearContents();
}

// -----------------------------------------------------------------------------

void VirconEmulator::PowerOn()
{
    // do nothing if power was already on
    if( PowerIsOn ) return;
    
    // turn on the console
    PowerIsOn = true;
    Reset();
}

// -----------------------------------------------------------------------------

void VirconEmulator::PowerOff()
{
    // do nothing if power was already off
    if( !PowerIsOn ) return;
    
    // turn off the console
    PowerIsOn = false;
    SPU.StopAllChannels();
}

// -----------------------------------------------------------------------------

void VirconEmulator::Pause()
{
    // do nothing when not applicable
    if( !PowerIsOn || Paused ) return;
    
    // take pause actions
    Paused = true;
    
    SPU.ThreadPauseFlag = true;
    alSourcePause( SPU.SoundSourceID );
}

// -----------------------------------------------------------------------------

void VirconEmulator::Resume()
{
    // do nothing when not applicable
    if( !PowerIsOn || !Paused ) return;
    
    // take resume actions
    Paused = false;
    
    alSourcePlay( SPU.SoundSourceID );
    SPU.ThreadPauseFlag = false;
}


// =============================================================================
//      VIRCON EMULATOR: EXTERNAL QUERIES
// =============================================================================


bool VirconEmulator::HasCartridge()
{
    return (CartridgeController.MemorySize != 0);
}

// -----------------------------------------------------------------------------

bool VirconEmulator::HasMemoryCard()
{
    return (MemoryCardController.MemorySize != 0);
}

// -----------------------------------------------------------------------------

bool VirconEmulator::HasGamepad( int Number )
{
    return GamepadController.IsGamepadConnected( Number );
}


// =============================================================================
//      EXTERNAL VOLUME CONTROL
// =============================================================================


float VirconEmulator::GetOutputVolume()
{
    float SPUVolume = SPU.OutputVolume;
    
    // within SPU output volume works linearly
    // (it is just a gain level) but here we
    // will treat it quadratically to get the
    // human-perceived output volume level
    // vary in a more progressive way
    Clamp( SPUVolume, 0, 1 );
    return sqrt( SPUVolume );
}

// -----------------------------------------------------------------------------

void VirconEmulator::SetOutputVolume( float Volume )
{
    // within SPU output volume works linearly
    // (it is just a gain level) but here we
    // will treat it quadratically to get the
    // human-perceived output volume level
    // vary in a more progressive way
    Clamp( Volume, 0, 1 );
    Volume = Volume * Volume;
    
    SPU.SetOutputVolume( Volume );
}

// -----------------------------------------------------------------------------

bool VirconEmulator::IsMuted()
{
    return SPU.Mute;
}

// -----------------------------------------------------------------------------

void VirconEmulator::SetMute( bool Mute )
{
    SPU.SetMute( Mute );
}


// =============================================================================
//      VIRCON EMULATOR: I/O FUNCTIONS
// =============================================================================


void VirconEmulator::ProcessEvent( SDL_Event Event )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Determine the type of event
    
    // ignore keypresses when control is pressed,
    // so that keyboard shortcuts will not interfere
    bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
    if( ControlIsPressed ) return;
    
    // save event information
    bool KeyIsPressed;
    SDL_Keycode KeyCode = Event.key.keysym.sym;
    
    if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
      KeyIsPressed = true;
    
    else if( Event.type == SDL_KEYUP )
      KeyIsPressed = false;
    
    // other events are not processed here
    else return;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Check our fixed mappings to gamepads
    
    // check keys for gamepad 1 directions
    if( KeyCode == SDLK_LEFT )
      GamepadController.ProcessDirectionChange( 0, GamepadDirections::Left, KeyIsPressed );
    
    if( KeyCode == SDLK_RIGHT )
      GamepadController.ProcessDirectionChange( 0, GamepadDirections::Right, KeyIsPressed );
    
    if( KeyCode == SDLK_UP )
      GamepadController.ProcessDirectionChange( 0, GamepadDirections::Up, KeyIsPressed );
    
    if( KeyCode == SDLK_DOWN )
      GamepadController.ProcessDirectionChange( 0, GamepadDirections::Down, KeyIsPressed );
    
    // check keys for gamepad 1 buttons
    if( KeyCode == SDLK_x )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::A, KeyIsPressed );
      
    if( KeyCode == SDLK_z )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::B, KeyIsPressed );
      
    if( KeyCode == SDLK_s )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::X, KeyIsPressed );
      
    if( KeyCode == SDLK_a )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::Y, KeyIsPressed );
      
    if( KeyCode == SDLK_q )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::L, KeyIsPressed );
      
    if( KeyCode == SDLK_w )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::R, KeyIsPressed );
    
    if( KeyCode == SDLK_SPACE )
      GamepadController.ProcessButtonChange( 0, GamepadButtons::Start, KeyIsPressed );
      
    // check keys for gamepad 2 directions
    if( KeyCode == SDLK_KP_4 )
      GamepadController.ProcessDirectionChange( 1, GamepadDirections::Left, KeyIsPressed );
    
    if( KeyCode == SDLK_KP_6 )
      GamepadController.ProcessDirectionChange( 1, GamepadDirections::Right, KeyIsPressed );
    
    if( KeyCode == SDLK_KP_8 )
      GamepadController.ProcessDirectionChange( 1, GamepadDirections::Up, KeyIsPressed );
    
    if( KeyCode == SDLK_KP_5 )
      GamepadController.ProcessDirectionChange( 1, GamepadDirections::Down, KeyIsPressed );
    
    // check keys for gamepad 2 buttons
    if( KeyCode == SDLK_m )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::A, KeyIsPressed );
      
    if( KeyCode == SDLK_n )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::B, KeyIsPressed );
      
    if( KeyCode == SDLK_j )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::X, KeyIsPressed );
      
    if( KeyCode == SDLK_h )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::Y, KeyIsPressed );
      
    if( KeyCode == SDLK_y )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::L, KeyIsPressed );
      
    if( KeyCode == SDLK_u )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::R, KeyIsPressed );
    
    if( KeyCode == SDLK_RETURN )
      GamepadController.ProcessButtonChange( 1, GamepadButtons::Start, KeyIsPressed );
}
