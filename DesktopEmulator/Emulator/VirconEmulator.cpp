// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconROMFormat.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/FilePaths.hpp"
    #include "../DesktopInfrastructure/LogStream.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    
    // include project headers
    #include "VirconEmulator.hpp"
    #include "Globals.hpp"
    #include "GUI.hpp"
    #include "Settings.hpp"
    
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
    
    // set initial state
    PowerIsOn = false;
    Paused = false;
    
    // initial loads are 0
    LastCPULoads[0] = LastCPULoads[1] = 0;
    LastGPULoads[0] = LastGPULoads[1] = 0;
    
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
    LOG_SCOPE( "Loading bios" );
    LOG( "File path: \"" << FilePath << "\"" );

    ifstream InputFile;
    InputFile.open( FilePath, ios_base::binary | ios_base::ate );
    
    if( InputFile.fail() )
      THROW( "Cannot open BIOS file" );
    
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
      THROW( "Input V32 ROM cannot be loaded as a BIOS (is it a cartridge instead)" );
    
    // now check the actual BIOS signature
    if( !CheckSignature( ROMHeader.Signature, Signatures::BiosFile ) )
      THROW( "Incorrect V32 file format (file does not have a valid signature)" );
    
    // check current Vircon version
    if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
    ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
      THROW( "This BIOS was made for a more recent version of Vircon. Please use an updated emulator" );
    
    // report the title
    ROMHeader.Title[ 63 ] = 0;
    LOG( "BIOS title: \"" << ROMHeader.Title << "\"" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Check the declared rom contents
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // ensure that there is exactly 1 texture
    if( ROMHeader.NumberOfTextures != 1 )
      THROW( "A BIOS video rom should have exactly 1 texture" );
    
    // ensure that there is exactly 1 sound
    if( ROMHeader.NumberOfSounds != 1 )
      THROW( "A BIOS audio rom should have exactly 1 sound" );
    
    // check for correct program rom location
    if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileHeader) )
      THROW( "Incorrect V32 file format (program ROM is not located after file header)" );
    
    // check for correct video rom location
    uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
    
    if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
      THROW( "Incorrect V32 file format (video ROM is not located after program ROM)" );
    
    // check for correct audio rom location
    uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
    
    if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
      THROW( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
    
    // check for correct file size
    uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
    
    if( FileBytes != SizeAfterAudioROM )
      THROW( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Load program rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // load a binary file header
    BinaryFileHeader BinaryHeader;
    InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileHeader) );
    
    // check signature for embedded binary
    if( !CheckSignature( BinaryHeader.Signature, Signatures::BinaryFile ) )
      THROW( "BIOS binary does not have a valid signature" );
    
    // checking program rom size limitations
    if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumBiosProgramROM ) )
      THROW( "BIOS binary does not have a correct size (from 1 word up to 1M words)" );
    
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
      THROW( "BIOS texture does not have a valid signature" );
    
    // report texture size
    LOG( "BIOS texture is " << TextureHeader.TextureWidth << "x" << TextureHeader.TextureHeight );
    
    // check texture size limitations
    if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
    ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
      THROW( "BIOS texture does not have correct dimensions (from 1x1 up to 1024x1024 pixels)" );
    
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
      THROW( "BIOS sound does not have a valid signature" );
    
    // report sound length
    LOG( "BIOS sound is " << SoundHeader.SoundSamples << " samples" );
    
    // check sound length limitations
    if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumBiosSamples ) )
      THROW( "BIOS sound does not have a correct length (from 1 up to 1M samples)" );
    
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
    LOG_SCOPE( "Loading cartridge" );
    LOG( "File path: \"" << FilePath << "\"" );

    // unload any previous cartridge
    UnloadCartridge();
    
    // open cartridge file
    ifstream InputFile;
    InputFile.open( FilePath, ios_base::binary | ios_base::ate );
    
    if( InputFile.fail() )
      THROW( "Cannot open cartridge file" );
    
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
      THROW( "Input V32 ROM cannot be loaded as a cartridge (is it a BIOS instead)" );
    
    // now check the actual cartridge signature
    if( !CheckSignature( ROMHeader.Signature, Signatures::CartridgeFile ) )
      THROW( "Incorrect V32 file format (file does not have a valid signature)" );
    
    // check current Vircon version
    if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
    ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
      THROW( "This cartridge was made for a more recent version of Vircon. Please use an updated emulator" );
    
    // report the title
    ROMHeader.Title[ 63 ] = 0;
    LOG( "Cartridge title: \"" << ROMHeader.Title << "\"" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Check the declared rom contents
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // check that there are not too many textures
    LOG( "Video ROM contains " << ROMHeader.NumberOfTextures << " textures" );
    
    if( ROMHeader.NumberOfTextures > (uint32_t)Constants::GPUMaximumCartridgeTextures )
      THROW( "Video ROM contains too many textures (Vircon GPU only allows up to 256)" );
    
    // check that there are not too many sounds
    LOG( "Audio ROM contains " << ROMHeader.NumberOfSounds << " sounds" );
    
    if( ROMHeader.NumberOfSounds > (uint32_t)Constants::SPUMaximumCartridgeSounds )
      THROW( "Audio ROM contains too many sounds (Vircon SPU only allows up to 1024)" );
    
    // check for correct program rom location
    if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileHeader) )
      THROW( "Incorrect V32 file format (program ROM is not located after file header)" );
    
    // check for correct video rom location
    uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
    
    if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
      THROW( "Incorrect V32 file format (video ROM is not located after program ROM)" );
    
    // check for correct audio rom location
    uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
    
    if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
      THROW( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
    
    // check for correct file size
    uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
    
    if( FileBytes != SizeAfterAudioROM )
      THROW( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Load program rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    {
        LOG_SCOPE( "Loading cartridge program ROM" );
        
        // load a binary file signature
        BinaryFileHeader BinaryHeader;
        InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileHeader) );
        
        // check signature for embedded binary
        if( !CheckSignature( BinaryHeader.Signature, Signatures::BinaryFile ) )
          THROW( "Cartridge binary does not have a valid signature" );
        
        LOG( "Program ROM is " << BinaryHeader.NumberOfWords << " words" );
        
        // check program rom size limitations
        if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumCartridgeProgramROM ) )
          THROW( "Cartridge program ROM does not have a correct size (from 1 word up to 128M words)" );
        
        // load the binary contents
        vector< VirconWord > LoadedBinary;
        LoadedBinary.resize( BinaryHeader.NumberOfWords );
        InputFile.read( (char*)(&LoadedBinary[0]), BinaryHeader.NumberOfWords*4 );
        CartridgeController.Connect( &LoadedBinary[0], BinaryHeader.NumberOfWords );
        
        // discard the temporary buffer
        LoadedBinary.clear();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Load video rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    {
        LOG_SCOPE( "Loading cartridge video ROM" );
        
        // load all textures in sequence
        for( unsigned i = 0; i < ROMHeader.NumberOfTextures; i++ )
        {
            // load a texture file signature
            TextureFileHeader TextureHeader;
            InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileHeader) );
            
            // check signature for embedded texture
            if( !CheckSignature( TextureHeader.Signature, Signatures::TextureFile ) )
              THROW( "Cartridge texture does not have a valid signature" );
            
            // report texture size
            LOG( "Texture " << i << ": " << TextureHeader.TextureWidth
                 << " x " << TextureHeader.TextureHeight << " pixels" );
            
            // check texture size limitations
            if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
            ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
              THROW( "Cartridge texture does not have correct dimensions (1x1 up to 1024x1024 pixels)" );
            
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
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: Load audio rom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    {
        LOG_SCOPE( "Loading cartridge audio ROM" );
        
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
              THROW( "Cartridge sound does not have a valid signature" );
            
            // report sound length
            LOG( "Sound " << i << ": " << SoundHeader.SoundSamples << " samples ("
                 << (SoundHeader.SoundSamples/44100.0f) << " seconds)" );
            
            // check length limitations for this sound
            if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumCartridgeSamples ) )
              THROW( "Cartridge sound does not have correct length (1 up to 256M samples)" );
            
            // check length limitations for the whole SPU
            TotalSPUSamples += SoundHeader.SoundSamples;
            
            if( TotalSPUSamples > (uint32_t)Constants::SPUMaximumCartridgeSamples )
              THROW( "Cartridge sounds contain too many total samples (Vircon SPU only allows up to 256M total samples)" );
            
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
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: General Vircon setup
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // only when loading was successful:
    // copy cartridge contents information
    CartridgeController.NumberOfTextures = ROMHeader.NumberOfTextures;
    CartridgeController.NumberOfSounds = ROMHeader.NumberOfSounds;
    
    // copy cartridge metadata
    CartridgeController.CartridgeTitle = ROMHeader.Title;
    CartridgeController.CartridgeVersion = ROMHeader.ROMVersion;
    CartridgeController.CartridgeRevision = ROMHeader.ROMRevision;
    
    // finally, close input file
    InputFile.close();
    
    // set window title
    string WindowTitle = string("Vircon32: ") + ROMHeader.Title;
    SDL_SetWindowTitle( OpenGL2D.Window, WindowTitle.c_str() );
    
    // update list of recent roms
    AddRecentCartridgePath( FilePath );
    
    // save the file name
    CartridgeController.CartridgeFileName = GetPathFileName( FilePath );
}

// -----------------------------------------------------------------------------

void VirconEmulator::UnloadCartridge()
{
    // do nothing if a cartridge is not loaded
    if( !HasCartridge() ) return;
    
    // release cartridge program ROM
    CartridgeController.Disconnect();
    CartridgeController.NumberOfTextures = 0;
    CartridgeController.NumberOfSounds = 0;
    
    // tell GPU to release all cartridge textures
    for( GPUTexture& T: GPU.CartridgeTextures )
      GPU.UnloadTexture( T );
    
    GPU.CartridgeTextures.clear();
    
    // tell SPU to release all cartridge sounds
    for( SPUSound& S: SPU.CartridgeSounds )
      SPU.UnloadSound( S );
    
    SPU.CartridgeSounds.clear();
    
    // set window title
    SDL_SetWindowTitle( OpenGL2D.Window, "Vircon32: No cartridge" );
}


// =============================================================================
//      VIRCON EMULATOR: MEMORY CARD MANAGEMENT
// =============================================================================


void VirconEmulator::CreateMemoryCard( const std::string& FilePath )
{
    LOG_SCOPE( "Creating memory card" );
    LOG( "File path: \"" << FilePath << "\"" );
    
    MemoryCardController.CreateNewFile( FilePath );
}

// -----------------------------------------------------------------------------

void VirconEmulator::LoadMemoryCard( const std::string& FilePath )
{
    LOG_SCOPE( "Loading memory card" );
    LOG( "File path: \"" << FilePath << "\"" );

    // unload any previous card
    UnloadMemoryCard();
    
    // load the card into memory
    Vircon.MemoryCardController.LoadContents( FilePath );
    
    // update list of recent cards
    AddRecentMemoryCardPath( FilePath );
}

// -----------------------------------------------------------------------------

void VirconEmulator::UnloadMemoryCard()
{
    // do nothing if a card is not loaded
    if( !HasMemoryCard() ) return;
    
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
    
    // after runnning the frame, update load info
    LastCPULoads[ 1 ] = LastCPULoads[ 0 ];
    LastCPULoads[ 0 ] = 100.0 * Timer.CycleCounter / Constants::CyclesPerFrame;
    
    int GPUUsedPixels = Constants::GPUPixelCapacityPerFrame - max( 0, GPU.RemainingPixels );
    LastGPULoads[ 1 ] = LastGPULoads[ 0 ];
    LastGPULoads[ 0 ] = 100.0 * GPUUsedPixels / Constants::GPUPixelCapacityPerFrame;
    
    // STEP 3: after running, ensure that all GPU
    // commands run in the current frame are drawn
    glFlush();
}

// -----------------------------------------------------------------------------

void VirconEmulator::Reset()
{
    LOG( "Emulator reset" );
    
    // first: transmit the message to all components that need it
    Timer.Reset();
    RNG.Reset();
    CPU.Reset();
    GPU.Reset();
    SPU.Reset();
    GamepadController.Reset();
    
    // now reset the emulator itself
    RAM.ClearContents();
    
    // loads become 0 on a reset
    LastCPULoads[0] = LastCPULoads[1] = 0;
    LastGPULoads[0] = LastGPULoads[1] = 0;
}

// -----------------------------------------------------------------------------

void VirconEmulator::PowerOn()
{
    LOG( "Emulator power ON" );
    
    // do nothing if power was already on
    if( PowerIsOn ) return;
    
    // turn on the console
    PowerIsOn = true;
    Reset();
}

// -----------------------------------------------------------------------------

void VirconEmulator::PowerOff()
{
    LOG( "Emulator power OFF" );
    
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
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // JOYSTICK CONNECTION EVENTS
    
    if( Event.type == SDL_JOYDEVICEADDED )
    {
        // access the joystick
        SDL_Joystick* NewJoystick = SDL_JoystickOpen( Event.jdevice.which );
        
        if( NewJoystick )
        {
            // find out joystick instance ID and GUID
            SDL_JoystickGUID NewGUID = SDL_JoystickGetGUID( NewJoystick );
            Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
            
            // update the list of connected joysticks
            ConnectedJoysticks[ AddedInstanceID ] = NewGUID;
        }
        
        // detect joysticks and assign them to gamepads
        AssignInputDevices();
    }
    
    else if( Event.type == SDL_JOYDEVICEREMOVED )
    {
        // find out joystick instance ID and GUID
        Sint32 RemovedInstanceID = Event.jdevice.which;
        SDL_Joystick* OldJoystick = SDL_JoystickFromInstanceID( RemovedInstanceID );
        
        // update the list of connected joysticks
        ConnectedJoysticks.erase( RemovedInstanceID );
        SDL_JoystickClose( OldJoystick );
        
        // detect joysticks and assign them to gamepads
        AssignInputDevices();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // KEYBOARD INPUT EVENTS
    
    else if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
    {
        SDL_Keycode KeyCode = Event.key.keysym.sym;
        
        // ignore keypresses when control is pressed,
        // so that keyboard shortcuts will not interfere
        bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
        if( ControlIsPressed ) return;
        
        // in other cases process the key normally
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is the keyboard
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
              continue;
            
            // check the mapped keys for directions
            if( KeyCode == KeyboardProfile.Left )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, true );
              
            if( KeyCode == KeyboardProfile.Right )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, true );
              
            if( KeyCode == KeyboardProfile.Up )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, true );
              
            if( KeyCode == KeyboardProfile.Down )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, true );
              
            // check the mapped keys for buttons
            if( KeyCode == KeyboardProfile.ButtonA )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, true );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, true );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, true );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, true );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, true );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, true );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, true );
        }
    }
    
    else if( Event.type == SDL_KEYUP )
    {
        SDL_Keycode KeyCode = Event.key.keysym.sym;
        
        // ignore keypresses when control is pressed,
        // so that keyboard shortcuts will not interfere
        bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
        if( ControlIsPressed ) return;
        
        // in other cases process the key normally
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is the keyboard
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
              continue;
            
            // check the mapped keys for directions
            if( KeyCode == KeyboardProfile.Left )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, false );
              
            if( KeyCode == KeyboardProfile.Right )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, false );
              
            if( KeyCode == KeyboardProfile.Up )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, false );
              
            if( KeyCode == KeyboardProfile.Down )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, false );
              
            // check the mapped keys for buttons
            if( KeyCode == KeyboardProfile.ButtonA )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, false );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, false );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, false );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, false );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, false );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, false );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, false );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // JOYSTICK INPUT EVENTS
    
    else if( Event.type == SDL_JOYAXISMOTION )
    {
        Uint8 AxisIndex = Event.jaxis.axis;
        Sint16 AxisPosition = Event.jaxis.value;
        Sint32 InstanceID = Event.jaxis.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        // we need to process both directions in this axis
        // at the same time, because they are correlated.
        // But be careful because it could happen that not
        // both directions have been mapped
        
        // joystick could be analog, so allow for
        // a dead zone in the center of +/- 50%
        bool PositivePressed = (AxisPosition > +16000);
        bool NegativePressed = (AxisPosition < -16000);
        
        // check all gamepads
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
              
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped axes for directions
            if( JoystickProfile->Left.IsAxis )
              if( AxisIndex == JoystickProfile->Left.AxisIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, JoystickProfile->Left.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Right.IsAxis )
              if( AxisIndex == JoystickProfile->Right.AxisIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, JoystickProfile->Right.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Up.IsAxis )
              if( AxisIndex == JoystickProfile->Up.AxisIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, JoystickProfile->Up.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Down.IsAxis )
              if( AxisIndex == JoystickProfile->Down.AxisIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, JoystickProfile->Down.AxisPositive? PositivePressed : NegativePressed );
            
            // check the mapped axes for buttons
            if( JoystickProfile->ButtonA.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonA.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, JoystickProfile->ButtonA.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonB.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonB.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, JoystickProfile->ButtonB.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonX.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonX.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, JoystickProfile->ButtonX.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonY.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonY.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, JoystickProfile->ButtonY.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonL.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonL.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, JoystickProfile->ButtonL.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonR.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonR.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, JoystickProfile->ButtonR.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonStart.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonStart.AxisIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, JoystickProfile->ButtonStart.AxisPositive? PositivePressed : NegativePressed );
        }
    }
    
    else if( Event.type == SDL_JOYBUTTONDOWN )
    {
        Uint8 ButtonIndex = Event.jbutton.button;
        Sint32 InstanceID = Event.jbutton.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped buttons for directions
            if( !JoystickProfile->Left.IsAxis )
              if( ButtonIndex == JoystickProfile->Left.ButtonIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, true );
              
            if( !JoystickProfile->Right.IsAxis )
              if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, true );
              
            if( !JoystickProfile->Up.IsAxis )
              if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, true );
              
            if( !JoystickProfile->Down.IsAxis )
              if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, true );
              
            // check the mapped buttons for buttons
            if( !JoystickProfile->ButtonA.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, true );
            
            if( !JoystickProfile->ButtonB.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, true );
            
            if( !JoystickProfile->ButtonX.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, true );
            
            if( !JoystickProfile->ButtonY.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, true );
              
            if( !JoystickProfile->ButtonL.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, true );
            
            if( !JoystickProfile->ButtonR.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, true );
            
            if( !JoystickProfile->ButtonStart.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, true );
        }
    }
    
    else if( Event.type == SDL_JOYBUTTONUP )
    {
        Uint8 ButtonIndex = Event.jbutton.button;
        Sint32 InstanceID = Event.jbutton.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped buttons for directions
            if( ButtonIndex == JoystickProfile->Left.ButtonIndex )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, false );
              
            if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, false );
              
            if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, false );
              
            if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
              GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, false );
              
            // check the mapped buttons for buttons
            if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, false );
            
            if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, false );
            
            if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, false );
            
            if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, false );
              
            if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, false );
            
            if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, false );
            
            if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
              GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, false );
        }
    }
    
    else if( Event.type == SDL_JOYHATMOTION )
    {
        Uint8 HatIndex = Event.jhat.hat;
        Uint8 HatDirection = Event.jhat.value;
        Sint32 InstanceID = Event.jhat.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        // we need to process both axes together, and
        // for each axis we need to process both directions
        
        // check all gamepads
        for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !GamepadController.IsGamepadConnected( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped axes for directions
            if( JoystickProfile->Left.IsHat )
              if( HatIndex == JoystickProfile->Left.HatIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Left, (bool)(HatDirection & JoystickProfile->Left.HatDirection) );
            
            if( JoystickProfile->Right.IsHat )
              if( HatIndex == JoystickProfile->Right.HatIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Right, (bool)(HatDirection & JoystickProfile->Right.HatDirection) );
            
            if( JoystickProfile->Up.IsHat )
              if( HatIndex == JoystickProfile->Up.HatIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Up, (bool)(HatDirection & JoystickProfile->Up.HatDirection) );
            
            if( JoystickProfile->Down.IsHat )
              if( HatIndex == JoystickProfile->Down.HatIndex )
                GamepadController.ProcessDirectionChange( Gamepad, GamepadDirections::Down, (bool)(HatDirection & JoystickProfile->Down.HatDirection) );
            
            // check the mapped buttons for buttons
            if( !JoystickProfile->ButtonA.IsHat )
              if( HatIndex == JoystickProfile->ButtonA.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::A, (bool)(HatDirection & JoystickProfile->ButtonA.HatDirection) );
            
            if( !JoystickProfile->ButtonB.IsHat )
              if( HatIndex == JoystickProfile->ButtonB.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::B, (bool)(HatDirection & JoystickProfile->ButtonB.HatDirection) );
            
            if( !JoystickProfile->ButtonX.IsHat )
              if( HatIndex == JoystickProfile->ButtonX.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::X, (bool)(HatDirection & JoystickProfile->ButtonX.HatDirection) );
            
            if( !JoystickProfile->ButtonY.IsHat )
              if( HatIndex == JoystickProfile->ButtonY.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Y, (bool)(HatDirection & JoystickProfile->ButtonY.HatDirection) );
            
            if( !JoystickProfile->ButtonL.IsHat )
              if( HatIndex == JoystickProfile->ButtonL.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::L, (bool)(HatDirection & JoystickProfile->ButtonL.HatDirection) );
            
            if( !JoystickProfile->ButtonR.IsHat )
              if( HatIndex == JoystickProfile->ButtonR.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::R, (bool)(HatDirection & JoystickProfile->ButtonR.HatDirection) );
            
            if( !JoystickProfile->ButtonStart.IsHat )
              if( HatIndex == JoystickProfile->ButtonStart.HatIndex )
                GamepadController.ProcessButtonChange( Gamepad, GamepadButtons::Start, (bool)(HatDirection & JoystickProfile->ButtonStart.HatDirection) );
        }
    }
}
