// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/NumericFunctions.hpp"
    #include "../DesktopInfrastructure/FilePaths.hpp"
    #include "../DesktopInfrastructure/FileSignatures.hpp"
    #include "../DesktopInfrastructure/Logger.hpp"
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    
    // include emulator headers
    #include "Globals.hpp"
    #include "GUI.hpp"
    #include "Settings.hpp"
    
    // include project headers
    #include "V32Console.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      V32 EMULATOR: INSTANCE HANDLING
    // =============================================================================
    
    
    V32Emulator::V32Emulator()
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
        LastCPULoads[ 0 ] = LastCPULoads[ 1 ] = 0;
        LastGPULoads[ 0 ] = LastGPULoads[ 1 ] = 0;
        
        // do NOT reset until power on
    }
    
    // -----------------------------------------------------------------------------
    
    V32Emulator::~V32Emulator()
    {
        // unload any present media
        if( HasMemoryCard() )  UnloadMemoryCard();
        if( HasCartridge() )   UnloadCartridge();
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: RESOURCE MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::Initialize()
    {
        // initialize audio playback
        SPU.InitializeAudio();
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::Terminate()
    {
        // terminate audio playback
        SPU.TerminateAudio();
        
        // release all connected media
        UnloadCartridge();
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::SetSPUSoundBuffers( int NumberOfBuffers )
    {
        SPU.NumberOfBuffers = NumberOfBuffers;
    }
    
    // -----------------------------------------------------------------------------
    
    int V32Emulator::GetSPUSoundBuffers()
    {
        return SPU.NumberOfBuffers;
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: EXTERNAL GENERAL OPERATION
    // =============================================================================
    
    
    void V32Emulator::Pause()
    {
        // do nothing when not applicable
        if( !PowerIsOn || Paused ) return;
        
        // take pause actions
        Paused = true;
        
        SPU.ThreadPauseFlag = true;
        alSourcePause( SPU.SoundSourceID );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::Resume()
    {
        // do nothing when not applicable
        if( !PowerIsOn || !Paused ) return;
        
        // take resume actions
        Paused = false;
        
        alSourcePlay( SPU.SoundSourceID );
        SPU.ThreadPauseFlag = false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::IsPaused()
    {
        return Paused;
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: BIOS MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::LoadBios( const std::string& FilePath )
    {
        // open bios file
        LOG( "Loading bios" );
        LOG( "File path: \"" + FilePath + "\"" );
    
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
          THROW( "Incorrect V32 file format (file size must be a multiple of 4)" );
        
        // ensure that we can at least load the file header
        if( FileBytes < sizeof(ROMFileFormat::Header) )
          THROW( "Incorrect V32 file format (file is too small)" );
        
        // now we can safely read the global header
        InputFile.seekg( 0, ios_base::beg );
        ROMFileFormat::Header ROMHeader;
        InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
        
        // check if the ROM is actually a cartridge
        if( CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
          THROW( "Input V32 ROM cannot be loaded as a BIOS (is it a cartridge instead)" );
        
        // now check the actual BIOS signature
        if( !CheckSignature( ROMHeader.Signature, ROMFileFormat::BiosSignature ) )
          THROW( "Incorrect V32 file format (file does not have a valid signature)" );
        
        // check current Vircon version
        if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
        ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
          THROW( "This BIOS was made for a more recent version of Vircon32. Please use an updated emulator" );
        
        // report the title
        ROMHeader.Title[ 63 ] = 0;
        LOG( string("BIOS title: \"") + ROMHeader.Title + "\"" );
        
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
        if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileFormat::Header) )
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
        BinaryFileFormat::Header BinaryHeader;
        InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
        
        // check signature for embedded binary
        if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
          THROW( "BIOS binary does not have a valid signature" );
        
        // checking program rom size limitations
        if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumBiosProgramROM ) )
          THROW( "BIOS binary does not have a correct size (from 1 word up to 1M words)" );
        
        // load the binary contents
        vector< V32Word > LoadedBinary;
        LoadedBinary.resize( BinaryHeader.NumberOfWords );
        InputFile.read( (char*)(&LoadedBinary[ 0 ]), BinaryHeader.NumberOfWords * 4 );
        BiosProgramROM.Connect( &LoadedBinary[ 0 ], BinaryHeader.NumberOfWords );
        
        // discard the temporary buffer
        LoadedBinary.clear();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Load video rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // load a texture file signature
        TextureFileFormat::Header TextureHeader;
        InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileFormat::Header) );
        
        // check signature for embedded texture
        if( !CheckSignature( TextureHeader.Signature, TextureFileFormat::Signature ) )
          THROW( "BIOS texture does not have a valid signature" );
        
        // report texture size
        LOG( "BIOS texture is " + to_string( TextureHeader.TextureWidth )
           + "x" + to_string( TextureHeader.TextureHeight ) );
        
        // check texture size limitations
        if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
        ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
          THROW( "BIOS texture does not have correct dimensions (from 1x1 up to 1024x1024 pixels)" );
        
        // load the texture pixels
        uint32_t TexturePixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
        vector< V32Word > LoadedTexture;
        LoadedTexture.resize( TexturePixels );
        InputFile.read( (char*)(&LoadedTexture[ 0 ]), TexturePixels * 4 );
        GPU.LoadTexture( GPU.BiosTexture, &LoadedTexture[ 0 ], TextureHeader.TextureWidth, TextureHeader.TextureHeight );
        
        // discard the temporary buffer
        LoadedTexture.clear();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 5: Load audio rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // load a sound file signature
        SoundFileFormat::Header SoundHeader;
        InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileFormat::Header) );
        
        // check signature for embedded sound
        if( !CheckSignature( SoundHeader.Signature, SoundFileFormat::Signature ) )
          THROW( "BIOS sound does not have a valid signature" );
        
        // report sound length
        LOG( "BIOS sound is " + to_string( SoundHeader.SoundSamples ) + " samples" );
        
        // check sound length limitations
        if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumBiosSamples ) )
          THROW( "BIOS sound does not have a correct length (from 1 up to 1M samples)" );
        
        // load the sound samples
        vector< SPUSample > LoadedSound;
        LoadedSound.resize( SoundHeader.SoundSamples );
        InputFile.read( (char*)(&LoadedSound[ 0 ]), SoundHeader.SoundSamples * 4 );
        SPU.LoadSound( SPU.BiosSound, &LoadedSound[ 0 ], SoundHeader.SoundSamples );
        
        // discard the temporary buffer
        LoadedSound.clear();
        
        // close the file
        InputFile.close();
        LOG( "Finished loading BIOS" );
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: CARTRIDGE MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::LoadCartridge( const std::string& FilePath )
    {
        LOG( "Loading cartridge" );
        LOG( "File path: \"" + FilePath + "\"" );
    
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
          THROW( "Incorrect V32 file format (file size must be a multiple of 4)" );
        
        // ensure that we can at least load the file header
        if( FileBytes < sizeof(ROMFileFormat::Header) )
          THROW( "Incorrect V32 file format (file is too small)" );
        
        // now we can safely read the global header
        InputFile.seekg( 0, ios_base::beg );
        ROMFileFormat::Header ROMHeader;
        InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
        
        // check if the ROM is actually a BIOS
        if( CheckSignature( ROMHeader.Signature, ROMFileFormat::BiosSignature ) )
          THROW( "Input V32 ROM cannot be loaded as a cartridge (is it a BIOS instead)" );
        
        // now check the actual cartridge signature
        if( !CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
          THROW( "Incorrect V32 file format (file does not have a valid signature)" );
        
        // check current Vircon version
        if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
        ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
          THROW( "This cartridge was made for a more recent version of Vircon32. Please use an updated emulator" );
        
        // report the title
        ROMHeader.Title[ 63 ] = 0;
        LOG( string("Cartridge title: \"") + ROMHeader.Title + "\"" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Check the declared rom contents
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // check that there are not too many textures
        LOG( "Video ROM contains " + to_string( ROMHeader.NumberOfTextures ) + " textures" );
        
        if( ROMHeader.NumberOfTextures > (uint32_t)Constants::GPUMaximumCartridgeTextures )
          THROW( "Video ROM contains too many textures (Vircon GPU only allows up to 256)" );
        
        // check that there are not too many sounds
        LOG( "Audio ROM contains " + to_string( ROMHeader.NumberOfSounds ) + " sounds" );
        
        if( ROMHeader.NumberOfSounds > (uint32_t)Constants::SPUMaximumCartridgeSounds )
          THROW( "Audio ROM contains too many sounds (Vircon SPU only allows up to 1024)" );
        
        // check for correct program rom location
        if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileFormat::Header) )
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
        
        LOG( "Loading cartridge program ROM" );
        
        // load a binary file signature
        BinaryFileFormat::Header BinaryHeader;
        InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
        
        // check signature for embedded binary
        if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
          THROW( "Cartridge binary does not have a valid signature" );
        
        LOG( "-> Program ROM is " + to_string( BinaryHeader.NumberOfWords ) + " words" );
        
        // check program rom size limitations
        if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumCartridgeProgramROM ) )
          THROW( "Cartridge program ROM does not have a correct size (from 1 word up to 128M words)" );
        
        // load the binary contents
        vector< V32Word > LoadedBinary;
        LoadedBinary.resize( BinaryHeader.NumberOfWords );
        InputFile.read( (char*)(&LoadedBinary[ 0 ]), BinaryHeader.NumberOfWords * 4 );
        CartridgeController.Connect( &LoadedBinary[ 0 ], BinaryHeader.NumberOfWords );
        
        // discard the temporary buffer
        LoadedBinary.clear();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 4: Load video rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        LOG( "Loading cartridge video ROM" );
        
        // load all textures in sequence
        for( unsigned i = 0; i < ROMHeader.NumberOfTextures; i++ )
        {
            // load a texture file signature
            TextureFileFormat::Header TextureHeader;
            InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileFormat::Header) );
            
            // check signature for embedded texture
            if( !CheckSignature( TextureHeader.Signature, TextureFileFormat::Signature ) )
              THROW( "Cartridge texture does not have a valid signature" );
            
            // report texture size
            LOG( "-> Texture " + to_string( i ) + ": " + to_string( TextureHeader.TextureWidth )
               + " x " + to_string( TextureHeader.TextureHeight ) + " pixels" );
            
            // check texture size limitations
            if( !IsBetween( TextureHeader.TextureWidth , 0, 1024 )
            ||  !IsBetween( TextureHeader.TextureHeight, 0, 1024 ) )
              THROW( "Cartridge texture does not have correct dimensions (1x1 up to 1024x1024 pixels)" );
            
            // load the texture pixels
            uint32_t TexturePixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
            vector< V32Word > LoadedTexture;
            LoadedTexture.resize( TexturePixels );
            InputFile.read( (char*)(&LoadedTexture[ 0 ]), TexturePixels * 4 );
            
            // create a new GPU texture and load data into it
            GPU.CartridgeTextures.emplace_back();
            GPU.LoadTexture( GPU.CartridgeTextures.back(), &LoadedTexture[ 0 ],
                             TextureHeader.TextureWidth, TextureHeader.TextureHeight );
            
            // discard the temporary buffer
            LoadedTexture.clear();
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 5: Load audio rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        LOG( "Loading cartridge audio ROM" );
        
        // keep count of the total sound samples
        uint32_t TotalSPUSamples = 0;
        
        // load all sounds in sequence
        for( unsigned i = 0; i < ROMHeader.NumberOfSounds; i++ )
        {
            // load a sound file signature
            SoundFileFormat::Header SoundHeader;
            InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileFormat::Header) );
            
            // check signature for embedded sound
            if( !CheckSignature( SoundHeader.Signature, SoundFileFormat::Signature ) )
              THROW( "Cartridge sound does not have a valid signature" );
            
            // report sound length
            LOG( "-> Sound " + to_string( i ) + ": " + to_string( SoundHeader.SoundSamples )
               + " samples (" + to_string( SoundHeader.SoundSamples/44100.0f ) + " seconds)" );
            
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
            InputFile.read( (char*)(&LoadedSound[ 0 ]), SoundHeader.SoundSamples * 4 );
            
            // create a new SPU sound and load data into it
            SPU.CartridgeSounds.emplace_back();
            SPU.LoadSound( SPU.CartridgeSounds.back(), &LoadedSound[ 0 ], SoundHeader.SoundSamples );
            
            // discard the temporary buffer
            LoadedSound.clear();
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
        LOG( "Finished loading cartridge" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::UnloadCartridge()
    {
        // do nothing if a cartridge is not loaded
        if( !HasCartridge() ) return;
        LOG( "Unloading cartridge" );
        
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
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::HasCartridge()
    {
        return (CartridgeController.MemorySize != 0);
    }
    
    // -----------------------------------------------------------------------------
    
    string V32Emulator::GetCartridgeFileName()
    {
        return CartridgeController.CartridgeFileName;
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: MEMORY CARD MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::CreateMemoryCard( const std::string& FilePath )
    {
        LOG( "Creating memory card" );
        LOG( "File path: \"" + FilePath + "\"" );
        
        // open the file
        ofstream OutputFile;
        OutputFile.open( FilePath, ios::binary | ios::trunc );
        
        if( OutputFile.fail() )
          THROW( "Cannot create memory card file" );
        
        // save the signature
        WriteSignature( OutputFile, MemoryCardFileFormat::Signature );
        
        // now save all empty contents
        vector< V32Word > EmptyWords;
        EmptyWords.resize( Constants::MemoryCardSize );
        OutputFile.write( (char*)(&EmptyWords[ 0 ]), Constants::MemoryCardSize * 4 );
        
        // close the file
        OutputFile.close();
        LOG( "Finished creating memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::LoadMemoryCard( const std::string& FilePath )
    {
        LOG( "Loading memory card" );
        LOG( "File path: \"" + FilePath + "\"" );
    
        // unload any previous card
        UnloadMemoryCard();
        
        // open the file for random access
        fstream& InputFile = MemoryCardController.LinkedFile;
        InputFile.open( FilePath, ios_base::in | ios_base::out | ios::binary | ios::ate );
        
        if( InputFile.fail() )
          THROW( "Cannot open memory card file" );
        
        // check file size coherency
        int NumberOfBytes = InputFile.tellg();
        int ExpectedBytes = 8 + Constants::MemoryCardSize * 4;
        
        if( NumberOfBytes != ExpectedBytes )
        {
            InputFile.close();
            THROW( "Invalid memory card: File does not match the size of a Vircon memory card" );
        }
        
        // read and check signature
        InputFile.seekg( 0, ios_base::beg );
        char FileSignature[ 8 ];
        InputFile.read( FileSignature, 8 );
        
        if( !CheckSignature( FileSignature, MemoryCardFileFormat::Signature ) )
          THROW( "Memory card file does not have a valid signature" );
        
        // connect the memory
        MemoryCardController.Connect( Constants::MemoryCardSize );
        
        // now load the whole memory card contents
        InputFile.read( (char*)(&MemoryCardController.Memory[ 0 ]), Constants::MemoryCardSize * 4 );
        
        // do NOT close the file! leave it open until
        // card is unloaded or emulation is stopped,
        // so that it can be saved if card is modified
        
        // update file name and list of recent cards
        MemoryCardController.CardFileName = GetPathFileName( FilePath );
        AddRecentMemoryCardPath( FilePath );
        
        LOG( "Finished loading memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::UnloadMemoryCard()
    {
        // do nothing if a card is not loaded
        if( !HasMemoryCard() ) return;
        LOG( "Unloading memory card" );
        
        // save the card if it was modified
        if( MemoryCardController.PendingSave )
          SaveMemoryCard();
        
        // remove the card memory
        MemoryCardController.Disconnect();
        
        // close the open file
        MemoryCardController.LinkedFile.close();
        LOG( "Finished unloading memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::SaveMemoryCard()
    {
        // do nothing if a card is not loaded
        if( !HasMemoryCard() ) return;
        
        // check the file
        fstream& OutputFile = MemoryCardController.LinkedFile;
        
        if( !OutputFile.is_open() || OutputFile.fail() )
          THROW( "Cannot save memory card file" );
        
        // save the signature
        OutputFile.seekp( ios_base::beg );
        WriteSignature( OutputFile, MemoryCardFileFormat::Signature );
        
        // now save all contents
        OutputFile.write( (char*)(&MemoryCardController.Memory[ 0 ]), Constants::MemoryCardSize * 4 );
        MemoryCardController.PendingSave = false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::HasMemoryCard()
    {
        return (MemoryCardController.MemorySize != 0);
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::WasMemoryCardModified()
    {
        return MemoryCardController.PendingSave;
    }
    
    // -----------------------------------------------------------------------------
    
    string V32Emulator::GetMemoryCardFileName()
    {
        return MemoryCardController.CardFileName;
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: CONTROL SIGNALS
    // =============================================================================
    
    
    void V32Emulator::SetPower( bool On )
    {
        // do nothing for no changes
        if( PowerIsOn == On ) return;
        PowerIsOn = On;
        
        // at power on, send an initial reset
        // to take care of initializations
        if( On )
        {
            LOG( "Emulator power ON" );
            Reset();
        }
        
        // at power off, stop all sound
        else
        {
            LOG( "Emulator power OFF" );
            SPU.StopAllChannels();
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::Reset()
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
        LastCPULoads[ 0 ] = LastCPULoads[ 1 ] = 0;
        LastGPULoads[ 0 ] = LastGPULoads[ 1 ] = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::RunNextFrame()
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
        
        // STEP 4: save memory card to file when modified
        if( MemoryCardController.PendingSave )
          SaveMemoryCard();
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: GENERAL STATUS QUERIES
    // =============================================================================
    
    
    bool V32Emulator::IsPowerOn()
    {
        return PowerIsOn;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::IsCPUHalted()
    {
        return CPU.Halted;
    }
    
    // -----------------------------------------------------------------------------
    
    // CPU load at the end of last frame, given in percentage
    float V32Emulator::GetCPULoad()
    {
        // take the maximum because in case of CPU overload,
        // there may be load of 100% every 2 frames (first
        // frame is not enough, and it finishes on next one)
        return max( LastCPULoads[ 0 ], Vircon.LastCPULoads[ 1 ] );
    }
    
    // -----------------------------------------------------------------------------
    
    // GPU load at the end of last frame, given in percentage
    float V32Emulator::GetGPULoad()
    {
        // same reasoning as for CPU load
        return max( LastGPULoads[ 0 ], Vircon.LastGPULoads[ 1 ] );
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: GAMEPAD MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::SetGamepadConnection( int GamepadPort, bool Connected )
    {
        // this function is just an external interface:
        // just pass the call to the gamepad controller
        GamepadController.SetGamepadConnection( GamepadPort, Connected );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::SetGamepadControl( int GamepadPort, GamepadControls Control, bool Pressed )  
    {
        // this function is just an external interface:
        // just pass the call to the gamepad controller
        GamepadController.SetGamepadControl( GamepadPort, Control, Pressed );
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Emulator::HasGamepad( int GamepadPort )
    {
        if( GamepadPort < 0 || GamepadPort >= Constants::GamepadPorts )
          return false;
        
        return GamepadController.RealTimeGamepadStates[ GamepadPort ].Connected;
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: TIMER MANAGEMENT
    // =============================================================================
    
    
    void V32Emulator::SetCurrentDate( int Year, int DaysWithinYear )
    {
        // input range checks
        Clamp( Year, 0, 32767 );
        Clamp( DaysWithinYear, 0, 365 );
        
        Timer.CurrentDate = (Year << 16) | DaysWithinYear;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::SetCurrentTime( int Hours, int Minutes, int Seconds )
    {
        // input range checks
        Clamp( Hours,   0, 23 );
        Clamp( Minutes, 0, 59 );
        Clamp( Seconds, 0, 59 );
        
        Timer.CurrentTime = (Hours * 3600) + (Minutes * 60) + Seconds;
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: EXTERNAL VOLUME CONTROL
    // =============================================================================
    
    
    float V32Emulator::GetOutputVolume()
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
    
    void V32Emulator::SetOutputVolume( float Volume )
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
    
    bool V32Emulator::IsMuted()
    {
        return SPU.Mute;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Emulator::SetMute( bool Mute )
    {
        SPU.SetMute( Mute );
    }
    
    
    // =============================================================================
    //      V32 EMULATOR: I/O FUNCTIONS
    // =============================================================================
    
    
    void V32Emulator::ProcessEvent( SDL_Event Event )
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
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
                  continue;
                
                // check if mapped device is the keyboard
                if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
                  continue;
                
                // check the mapped keys for directions
                if( KeyCode == KeyboardProfile.Left )
                  SetGamepadControl( Gamepad, GamepadControls::Left, true );
                  
                if( KeyCode == KeyboardProfile.Right )
                  SetGamepadControl( Gamepad, GamepadControls::Right, true );
                  
                if( KeyCode == KeyboardProfile.Up )
                  SetGamepadControl( Gamepad, GamepadControls::Up, true );
                  
                if( KeyCode == KeyboardProfile.Down )
                  SetGamepadControl( Gamepad, GamepadControls::Down, true );
                  
                // check the mapped keys for buttons
                if( KeyCode == KeyboardProfile.ButtonA )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonA, true );
                
                if( KeyCode == KeyboardProfile.ButtonB )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonB, true );
                
                if( KeyCode == KeyboardProfile.ButtonX )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonX, true );
                
                if( KeyCode == KeyboardProfile.ButtonY )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonY, true );
                  
                if( KeyCode == KeyboardProfile.ButtonL )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonL, true );
                
                if( KeyCode == KeyboardProfile.ButtonR )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonR, true );
                
                if( KeyCode == KeyboardProfile.ButtonStart )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonStart, true );
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
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
                  continue;
                
                // check if mapped device is the keyboard
                if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
                  continue;
                
                // check the mapped keys for directions
                if( KeyCode == KeyboardProfile.Left )
                  SetGamepadControl( Gamepad, GamepadControls::Left, false );
                  
                if( KeyCode == KeyboardProfile.Right )
                  SetGamepadControl( Gamepad, GamepadControls::Right, false );
                  
                if( KeyCode == KeyboardProfile.Up )
                  SetGamepadControl( Gamepad, GamepadControls::Up, false );
                  
                if( KeyCode == KeyboardProfile.Down )
                  SetGamepadControl( Gamepad, GamepadControls::Down, false );
                  
                // check the mapped keys for buttons
                if( KeyCode == KeyboardProfile.ButtonA )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonA, false );
                
                if( KeyCode == KeyboardProfile.ButtonB )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonB, false );
                
                if( KeyCode == KeyboardProfile.ButtonX )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonX, false );
                
                if( KeyCode == KeyboardProfile.ButtonY )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonY, false );
                  
                if( KeyCode == KeyboardProfile.ButtonL )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonL, false );
                
                if( KeyCode == KeyboardProfile.ButtonR )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonR, false );
                
                if( KeyCode == KeyboardProfile.ButtonStart )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonStart, false );
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
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
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
                    SetGamepadControl( Gamepad, GamepadControls::Left, JoystickProfile->Left.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->Right.IsAxis )
                  if( AxisIndex == JoystickProfile->Right.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Right, JoystickProfile->Right.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->Up.IsAxis )
                  if( AxisIndex == JoystickProfile->Up.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Up, JoystickProfile->Up.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->Down.IsAxis )
                  if( AxisIndex == JoystickProfile->Down.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Down, JoystickProfile->Down.AxisPositive? PositivePressed : NegativePressed );
                
                // check the mapped axes for buttons
                if( JoystickProfile->ButtonA.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonA.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonA, JoystickProfile->ButtonA.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonB.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonB.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonB, JoystickProfile->ButtonB.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonX.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonX.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonX, JoystickProfile->ButtonX.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonY.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonY.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonY, JoystickProfile->ButtonY.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonL.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonL.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonL, JoystickProfile->ButtonL.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonR.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonR.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonR, JoystickProfile->ButtonR.AxisPositive? PositivePressed : NegativePressed );
                
                if( JoystickProfile->ButtonStart.IsAxis )
                  if( AxisIndex == JoystickProfile->ButtonStart.AxisIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonStart, JoystickProfile->ButtonStart.AxisPositive? PositivePressed : NegativePressed );
            }
        }
        
        else if( Event.type == SDL_JOYBUTTONDOWN )
        {
            Uint8 ButtonIndex = Event.jbutton.button;
            Sint32 InstanceID = Event.jbutton.which;
            SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
            SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
            
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
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
                    SetGamepadControl( Gamepad, GamepadControls::Left, true );
                  
                if( !JoystickProfile->Right.IsAxis )
                  if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Right, true );
                  
                if( !JoystickProfile->Up.IsAxis )
                  if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Up, true );
                  
                if( !JoystickProfile->Down.IsAxis )
                  if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Down, true );
                  
                // check the mapped buttons for buttons
                if( !JoystickProfile->ButtonA.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonA, true );
                
                if( !JoystickProfile->ButtonB.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonB, true );
                
                if( !JoystickProfile->ButtonX.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonX, true );
                
                if( !JoystickProfile->ButtonY.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonY, true );
                  
                if( !JoystickProfile->ButtonL.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonL, true );
                
                if( !JoystickProfile->ButtonR.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonR, true );
                
                if( !JoystickProfile->ButtonStart.IsAxis )
                  if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonStart, true );
            }
        }
        
        else if( Event.type == SDL_JOYBUTTONUP )
        {
            Uint8 ButtonIndex = Event.jbutton.button;
            Sint32 InstanceID = Event.jbutton.which;
            SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
            SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
            
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
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
                  SetGamepadControl( Gamepad, GamepadControls::Left, false );
                  
                if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::Right, false );
                  
                if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::Up, false );
                  
                if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::Down, false );
                  
                // check the mapped buttons for buttons
                if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonA, false );
                
                if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonB, false );
                
                if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonX, false );
                
                if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonY, false );
                  
                if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonL, false );
                
                if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonR, false );
                
                if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
                  SetGamepadControl( Gamepad, GamepadControls::ButtonStart, false );
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
            for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
            {
                // non-connected gamepads are ignored
                if( !HasGamepad( Gamepad ) )
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
                    SetGamepadControl( Gamepad, GamepadControls::Left, (bool)(HatDirection & JoystickProfile->Left.HatDirection) );
                
                if( JoystickProfile->Right.IsHat )
                  if( HatIndex == JoystickProfile->Right.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Right, (bool)(HatDirection & JoystickProfile->Right.HatDirection) );
                
                if( JoystickProfile->Up.IsHat )
                  if( HatIndex == JoystickProfile->Up.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Up, (bool)(HatDirection & JoystickProfile->Up.HatDirection) );
                
                if( JoystickProfile->Down.IsHat )
                  if( HatIndex == JoystickProfile->Down.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::Down, (bool)(HatDirection & JoystickProfile->Down.HatDirection) );
                
                // check the mapped buttons for buttons
                if( !JoystickProfile->ButtonA.IsHat )
                  if( HatIndex == JoystickProfile->ButtonA.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonA, (bool)(HatDirection & JoystickProfile->ButtonA.HatDirection) );
                
                if( !JoystickProfile->ButtonB.IsHat )
                  if( HatIndex == JoystickProfile->ButtonB.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonB, (bool)(HatDirection & JoystickProfile->ButtonB.HatDirection) );
                
                if( !JoystickProfile->ButtonX.IsHat )
                  if( HatIndex == JoystickProfile->ButtonX.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonX, (bool)(HatDirection & JoystickProfile->ButtonX.HatDirection) );
                
                if( !JoystickProfile->ButtonY.IsHat )
                  if( HatIndex == JoystickProfile->ButtonY.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonY, (bool)(HatDirection & JoystickProfile->ButtonY.HatDirection) );
                
                if( !JoystickProfile->ButtonL.IsHat )
                  if( HatIndex == JoystickProfile->ButtonL.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonL, (bool)(HatDirection & JoystickProfile->ButtonL.HatDirection) );
                
                if( !JoystickProfile->ButtonR.IsHat )
                  if( HatIndex == JoystickProfile->ButtonR.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonR, (bool)(HatDirection & JoystickProfile->ButtonR.HatDirection) );
                
                if( !JoystickProfile->ButtonStart.IsHat )
                  if( HatIndex == JoystickProfile->ButtonStart.HatIndex )
                    SetGamepadControl( Gamepad, GamepadControls::ButtonStart, (bool)(HatDirection & JoystickProfile->ButtonStart.HatDirection) );
            }
        }
    }
}
