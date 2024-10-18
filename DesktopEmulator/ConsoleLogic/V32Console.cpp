// *****************************************************************************
    // include common Vircon32 headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/FileFormats.hpp"
    
    // include console logic headers
    #include "V32Console.hpp"
    #include "ExternalInterfaces.hpp"
    #include "AuxiliaryFunctions.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    
    // these are only needed to treat UTF-16 file paths
    #if defined(__WIN32__)
      #include <locale>         // [ C++ STL ] Locales
      #include <codecvt>        // [ C++ STL ] Encoding conversions
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // buffer used to transmit textures from loaded ROM files to the video library
    static GPUColor LoadedTexture[ Constants::GPUTextureSize ][ Constants::GPUTextureSize ];
    
    
    // =============================================================================
    //      V32 CONSOLE: INSTANCE HANDLING
    // =============================================================================
    
    
    V32Console::V32Console()
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
        
        // initial loads are 0
        LastCPULoads[ 0 ] = LastCPULoads[ 1 ] = 0;
        LastGPULoads[ 0 ] = LastGPULoads[ 1 ] = 0;
        
        // do NOT reset until power on
    }
    
    // -----------------------------------------------------------------------------
    
    V32Console::~V32Console()
    {
        // unload any present media
        if( HasMemoryCard() )  UnloadMemoryCard();
        if( HasCartridge() )   UnloadCartridge();
        if( HasBios() )        UnloadBios();
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: CONTROL SIGNALS
    // =============================================================================
    
    
    void V32Console::SetPower( bool On )
    {
        // do nothing for no changes
        if( PowerIsOn == On ) return;
        PowerIsOn = On;
        
        // at power on, send an initial reset
        // to take care of initializations
        if( On )
        {
            Callbacks::LogLine( "Console power ON" );
            Reset();
        }
        
        // at power off, stop all sound
        else
        {
            Callbacks::LogLine( "Console power OFF" );
            SPU.StopAllChannels();
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::Reset()
    {
        Callbacks::LogLine( "Console reset" );
        
        // first: transmit the message to all components that need it
        Timer.Reset();
        RNG.Reset();
        CPU.Reset();
        GPU.Reset();
        SPU.Reset();
        GamepadController.Reset();
        
        // now reset the console itself
        RAM.ClearContents();
        
        // loads become 0 on a reset
        LastCPULoads[ 0 ] = LastCPULoads[ 1 ] = 0;
        LastGPULoads[ 0 ] = LastGPULoads[ 1 ] = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::RunNextFrame()
    {
        // do nothing when not applicable
        if( !PowerIsOn )
          return;
        
        // STEP 1: Begin a new frame by sending
        // a frame change message to components
        Timer.ChangeFrame();
        CPU.ChangeFrame();
        GPU.ChangeFrame();
        SPU.ChangeFrame();
        GamepadController.ChangeFrame();
        
        // STEP 2: Run a frame's worth of cycles
        try
        {
            for( int i = 0; i < Constants::CyclesPerFrame; i++ )
            {
                // end loop early when CPU is set to wait
                if( CPU.Waiting || CPU.Halted )
                  break;
                
                // only these components need to
                // be notified of each CPU cycle
                Timer.RunNextCycle();
                CPU.RunNextCycle();
            }
        }
        catch( CPUException& CPUex )
        {
            // do nothing: the only purpose of these exceptions
            // is to stop the loop without checking in every step
        }
        
        // after runnning the frame, update load info
        LastCPULoads[ 1 ] = LastCPULoads[ 0 ];
        LastCPULoads[ 0 ] = 100.0 * Timer.CycleCounter / Constants::CyclesPerFrame;
        
        int GPUUsedPixels = Constants::GPUPixelCapacityPerFrame - max( 0, GPU.RemainingPixels );
        LastGPULoads[ 1 ] = LastGPULoads[ 0 ];
        LastGPULoads[ 0 ] = 100.0 * GPUUsedPixels / Constants::GPUPixelCapacityPerFrame;
        
        // STEP 3: save memory card to file when modified
        if( MemoryCardController.PendingSave )
          SaveMemoryCard();
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: GENERAL STATUS QUERIES
    // =============================================================================
    
    
    bool V32Console::IsPowerOn()
    {
        return PowerIsOn;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::IsCPUHalted()
    {
        return CPU.Halted;
    }
    
    // -----------------------------------------------------------------------------
    
    // CPU load at the end of last frame, given in percentage
    float V32Console::GetCPULoad()
    {
        // take the maximum because in case of CPU overload,
        // there may be load of 100% every 2 frames (first
        // frame is not enough, and it finishes on next one)
        return max( LastCPULoads[ 0 ], LastCPULoads[ 1 ] );
    }
    
    // -----------------------------------------------------------------------------
    
    // GPU load at the end of last frame, given in percentage
    float V32Console::GetGPULoad()
    {
        // same reasoning as for CPU load
        return max( LastGPULoads[ 0 ], LastGPULoads[ 1 ] );
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: BIOS MANAGEMENT
    // =============================================================================
    
    
    void V32Console::LoadBios( const std::string& FilePath )
    {
        Callbacks::LogLine( "Loading bios" );
        Callbacks::LogLine( "File path: \"" + FilePath + "\"" );
        
        // unload any previous bios
        UnloadBios();
        
        // open bios file
        ifstream InputFile;
        
        // on windows convert path from UTF-8 to UTF-16
        #if defined(__WIN32__)
          wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
          wstring FilePathUTF16 = converter.from_bytes(FilePath);
          InputFile.open( FilePathUTF16.c_str(), ios_base::binary | ios_base::ate );
        #else
          InputFile.open( FilePath, ios_base::binary | ios_base::ate );
        #endif
        
        if( InputFile.fail() )
          Callbacks::ThrowException( "Cannot open BIOS file" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load global information
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // get size and ensure it is a multiple of 4
        // (otherwise file contents are wrong)
        unsigned FileBytes = InputFile.tellg();
        
        if( (FileBytes % 4) != 0 )
          Callbacks::ThrowException( "Incorrect V32 file format (file size must be a multiple of 4)" );
        
        // ensure that we can at least load the file header
        if( FileBytes < sizeof(ROMFileFormat::Header) )
          Callbacks::ThrowException( "Incorrect V32 file format (file is too small)" );
        
        // now we can safely read the global header
        InputFile.seekg( 0, ios_base::beg );
        ROMFileFormat::Header ROMHeader;
        InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
        
        // check if the ROM is actually a cartridge
        if( CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
          Callbacks::ThrowException( "Input V32 ROM cannot be loaded as a BIOS (is it a cartridge instead)" );
        
        // now check the actual BIOS signature
        if( !CheckSignature( ROMHeader.Signature, ROMFileFormat::BiosSignature ) )
          Callbacks::ThrowException( "Incorrect V32 file format (file does not have a valid signature)" );
        
        // check current Vircon version
        if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
        ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
          Callbacks::ThrowException( "This BIOS was made for a more recent version of Vircon32. Please use an updated emulator" );
        
        // report the title
        ROMHeader.Title[ 63 ] = 0;
        Callbacks::LogLine( string("BIOS title: \"") + ROMHeader.Title + "\"" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Check the declared rom contents
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // ensure that there is exactly 1 texture
        if( ROMHeader.NumberOfTextures != 1 )
          Callbacks::ThrowException( "A BIOS video rom should have exactly 1 texture" );
        
        // ensure that there is exactly 1 sound
        if( ROMHeader.NumberOfSounds != 1 )
          Callbacks::ThrowException( "A BIOS audio rom should have exactly 1 sound" );
        
        // check for correct program rom location
        if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileFormat::Header) )
          Callbacks::ThrowException( "Incorrect V32 file format (program ROM is not located after file header)" );
        
        // check for correct video rom location
        uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
        
        if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
          Callbacks::ThrowException( "Incorrect V32 file format (video ROM is not located after program ROM)" );
        
        // check for correct audio rom location
        uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
        
        if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
          Callbacks::ThrowException( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
        
        // check for correct file size
        uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
        
        if( FileBytes != SizeAfterAudioROM )
          Callbacks::ThrowException( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: Load program rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // load a binary file header
        BinaryFileFormat::Header BinaryHeader;
        InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
        
        // check signature for embedded binary
        if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
          Callbacks::ThrowException( "BIOS binary does not have a valid signature" );
        
        // checking program rom size limitations
        if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumBiosProgramROM ) )
          Callbacks::ThrowException( "BIOS binary does not have a correct size (from 1 word up to 1M words)" );
        
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
          Callbacks::ThrowException( "BIOS texture does not have a valid signature" );
        
        // report texture size
        Callbacks::LogLine( "BIOS texture is " + to_string( TextureHeader.TextureWidth )
           + "x" + to_string( TextureHeader.TextureHeight ) );
        
        // check texture size limitations
        if( !IsBetween( TextureHeader.TextureWidth , 1, Constants::GPUTextureSize )
        ||  !IsBetween( TextureHeader.TextureHeight, 1, Constants::GPUTextureSize ) )
          Callbacks::ThrowException( "BIOS texture does not have correct dimensions (from 1x1 up to 1024x1024 pixels)" );
        
        // clear all texture pixels
        memset( LoadedTexture, 0, sizeof(LoadedTexture) );
        
        // load the texture pixels line by line,
        // in order to expand it to full size
        for( unsigned y = 0; y < TextureHeader.TextureHeight; y++ )
          InputFile.read( (char*)(LoadedTexture[ y ]), TextureHeader.TextureWidth * 4 );
        
        // send bios texture to the video library
        Callbacks::LoadTexture( -1, LoadedTexture );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 5: Load audio rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // load a sound file signature
        SoundFileFormat::Header SoundHeader;
        InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileFormat::Header) );
        
        // check signature for embedded sound
        if( !CheckSignature( SoundHeader.Signature, SoundFileFormat::Signature ) )
          Callbacks::ThrowException( "BIOS sound does not have a valid signature" );
        
        // report sound length
        Callbacks::LogLine( "BIOS sound is " + to_string( SoundHeader.SoundSamples ) + " samples" );
        
        // check sound length limitations
        if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumBiosSamples ) )
          Callbacks::ThrowException( "BIOS sound does not have a correct length (from 1 up to 1M samples)" );
        
        // load the sound samples
        vector< SPUSample > LoadedSound;
        LoadedSound.resize( SoundHeader.SoundSamples );
        InputFile.read( (char*)(&LoadedSound[ 0 ]), SoundHeader.SoundSamples * 4 );
        SPU.LoadSound( SPU.BiosSound, &LoadedSound[ 0 ], SoundHeader.SoundSamples );
        
        // discard the temporary buffer
        LoadedSound.clear();
        
        // only when loading was successful:
        // copy BIOS metadata
        BiosFileName = GetPathFileName( FilePath );
        BiosTitle = ROMHeader.Title;
        BiosVersion = ROMHeader.ROMVersion;
        BiosRevision = ROMHeader.ROMRevision;
        
        // close the file and report success
        InputFile.close();
        Callbacks::LogLine( "Finished loading BIOS" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::UnloadBios()
    {
        // do nothing if a bios is not loaded
        if( !HasBios() ) return;
        Callbacks::LogLine( "Unloading bios" );
        
        // release bios program ROM
        BiosProgramROM.Disconnect();
        BiosFileName = "";
        BiosTitle = "";
        BiosVersion = 0;
        BiosRevision = 0;
        
        // release the bios texture
        Callbacks::UnloadBiosTexture();
        
        // tell SPU to release the bios sounds
        SPU.UnloadSound( SPU.BiosSound );
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::HasBios()
    {
        return (BiosProgramROM.MemorySize != 0);
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: CARTRIDGE MANAGEMENT
    // =============================================================================
    
    
    void V32Console::LoadCartridge( const std::string& FilePath )
    {
        Callbacks::LogLine( "Loading cartridge" );
        Callbacks::LogLine( "File path: \"" + FilePath + "\"" );
    
        // unload any previous cartridge
        UnloadCartridge();
        
        // open cartridge file
        ifstream InputFile;
        
        // on windows convert path from UTF-8 to UTF-16
        #if defined(__WIN32__)
          wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
          wstring FilePathUTF16 = converter.from_bytes(FilePath);
          InputFile.open( FilePathUTF16.c_str(), ios_base::binary | ios_base::ate );
        #else
          InputFile.open( FilePath, ios_base::binary | ios_base::ate );
        #endif
        
        if( InputFile.fail() )
          Callbacks::ThrowException( "Cannot open cartridge file" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Load global information
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // get size and ensure it is a multiple of 4
        // (otherwise file contents are wrong)
        unsigned FileBytes = InputFile.tellg();
        
        if( (FileBytes % 4) != 0 )
          Callbacks::ThrowException( "Incorrect V32 file format (file size must be a multiple of 4)" );
        
        // ensure that we can at least load the file header
        if( FileBytes < sizeof(ROMFileFormat::Header) )
          Callbacks::ThrowException( "Incorrect V32 file format (file is too small)" );
        
        // now we can safely read the global header
        InputFile.seekg( 0, ios_base::beg );
        ROMFileFormat::Header ROMHeader;
        InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
        
        // check if the ROM is actually a BIOS
        if( CheckSignature( ROMHeader.Signature, ROMFileFormat::BiosSignature ) )
          Callbacks::ThrowException( "Input V32 ROM cannot be loaded as a cartridge (is it a BIOS instead)" );
        
        // now check the actual cartridge signature
        if( !CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
          Callbacks::ThrowException( "Incorrect V32 file format (file does not have a valid signature)" );
        
        // check current Vircon version
        if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
        ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
          Callbacks::ThrowException( "This cartridge was made for a more recent version of Vircon32. Please use an updated emulator" );
        
        // report the title
        ROMHeader.Title[ 63 ] = 0;
        Callbacks::LogLine( string("Cartridge title: \"") + ROMHeader.Title + "\"" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Check the declared rom contents
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // check that there are not too many textures
        Callbacks::LogLine( "Video ROM contains " + to_string( ROMHeader.NumberOfTextures ) + " textures" );
        
        if( ROMHeader.NumberOfTextures > (uint32_t)Constants::GPUMaximumCartridgeTextures )
          Callbacks::ThrowException( "Video ROM contains too many textures (Vircon GPU only allows up to 256)" );
        
        // check that there are not too many sounds
        Callbacks::LogLine( "Audio ROM contains " + to_string( ROMHeader.NumberOfSounds ) + " sounds" );
        
        if( ROMHeader.NumberOfSounds > (uint32_t)Constants::SPUMaximumCartridgeSounds )
          Callbacks::ThrowException( "Audio ROM contains too many sounds (Vircon SPU only allows up to 1024)" );
        
        // check for correct program rom location
        if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileFormat::Header) )
          Callbacks::ThrowException( "Incorrect V32 file format (program ROM is not located after file header)" );
        
        // check for correct video rom location
        uint32_t SizeAfterProgramROM = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
        
        if( ROMHeader.VideoROMLocation.StartOffset != SizeAfterProgramROM )
          Callbacks::ThrowException( "Incorrect V32 file format (video ROM is not located after program ROM)" );
        
        // check for correct audio rom location
        uint32_t SizeAfterVideoROM = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
        
        if( ROMHeader.AudioROMLocation.StartOffset != SizeAfterVideoROM )
          Callbacks::ThrowException( "Incorrect V32 file format (audio ROM is not located after video ROM)" );
        
        // check for correct file size
        uint32_t SizeAfterAudioROM = ROMHeader.AudioROMLocation.StartOffset + ROMHeader.AudioROMLocation.Length;
        
        if( FileBytes != SizeAfterAudioROM )
          Callbacks::ThrowException( "Incorrect V32 file format (file size does not match indicated ROM contents)" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 3: Load program rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        Callbacks::LogLine( "Loading cartridge program ROM" );
        
        // load a binary file signature
        BinaryFileFormat::Header BinaryHeader;
        InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
        
        // check signature for embedded binary
        if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
          Callbacks::ThrowException( "Cartridge binary does not have a valid signature" );
        
        Callbacks::LogLine( "-> Program ROM is " + to_string( BinaryHeader.NumberOfWords ) + " words" );
        
        // check program rom size limitations
        if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumCartridgeProgramROM ) )
          Callbacks::ThrowException( "Cartridge program ROM does not have a correct size (from 1 word up to 128M words)" );
        
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
        
        Callbacks::LogLine( "Loading cartridge video ROM" );
        
        // load all textures in sequence
        for( unsigned i = 0; i < ROMHeader.NumberOfTextures; i++ )
        {
            // load a texture file signature
            TextureFileFormat::Header TextureHeader;
            InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileFormat::Header) );
            
            // check signature for embedded texture
            if( !CheckSignature( TextureHeader.Signature, TextureFileFormat::Signature ) )
              Callbacks::ThrowException( "Cartridge texture does not have a valid signature" );
            
            // report texture size
            Callbacks::LogLine( "-> Texture " + to_string( i ) + ": " + to_string( TextureHeader.TextureWidth )
               + " x " + to_string( TextureHeader.TextureHeight ) + " pixels" );
            
            // check texture size limitations
            if( !IsBetween( TextureHeader.TextureWidth , 1, Constants::GPUTextureSize )
            ||  !IsBetween( TextureHeader.TextureHeight, 1, Constants::GPUTextureSize ) )
              Callbacks::ThrowException( "Cartridge texture does not have correct dimensions (1x1 up to 1024x1024 pixels)" );
            
            // clear all texture pixels
            memset( LoadedTexture, 0, sizeof(LoadedTexture) );
            
            // load the texture pixels line by line,
            // in order to expand it to full size
            for( unsigned y = 0; y < TextureHeader.TextureHeight; y++ )
              InputFile.read( (char*)(LoadedTexture[ y ]), TextureHeader.TextureWidth * 4 );
            
            // send this texture to the video library
            Callbacks::LoadTexture( i, LoadedTexture );
        }
        
        // now update GPU with the inserted textures
        GPU.InsertCartridgeTextures( ROMHeader.NumberOfTextures );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 5: Load audio rom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        Callbacks::LogLine( "Loading cartridge audio ROM" );
        
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
              Callbacks::ThrowException( "Cartridge sound does not have a valid signature" );
            
            // report sound length
            Callbacks::LogLine( "-> Sound " + to_string( i ) + ": " + to_string( SoundHeader.SoundSamples )
               + " samples (" + to_string( SoundHeader.SoundSamples/44100.0f ) + " seconds)" );
            
            // check length limitations for this sound
            if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumCartridgeSamples ) )
              Callbacks::ThrowException( "Cartridge sound does not have correct length (1 up to 256M samples)" );
            
            // check length limitations for the whole SPU
            TotalSPUSamples += SoundHeader.SoundSamples;
            
            if( TotalSPUSamples > (uint32_t)Constants::SPUMaximumCartridgeSamples )
              Callbacks::ThrowException( "Cartridge sounds contain too many total samples (Vircon SPU only allows up to 256M total samples)" );
            
            // load the sound samples
            vector< SPUSample > LoadedSound;
            LoadedSound.resize( SoundHeader.SoundSamples );
            InputFile.read( (char*)(&LoadedSound[ 0 ]), SoundHeader.SoundSamples * 4 );
            
            // create a new SPU sound and load data into it
            SPU.LoadSound( SPU.CartridgeSounds[ i ], &LoadedSound[ 0 ], SoundHeader.SoundSamples );
            
            // discard the temporary buffer
            LoadedSound.clear();
        }
        
        SPU.LoadedCartridgeSounds = ROMHeader.NumberOfSounds;
        
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
        
        // save the file name
        CartridgeController.CartridgeFileName = GetPathFileName( FilePath );
        Callbacks::LogLine( "Finished loading cartridge" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::UnloadCartridge()
    {
        // do nothing if a cartridge is not loaded
        if( !HasCartridge() ) return;
        Callbacks::LogLine( "Unloading cartridge" );
        
        // release cartridge program ROM
        CartridgeController.Disconnect();
        CartridgeController.NumberOfTextures = 0;
        CartridgeController.NumberOfSounds = 0;
        CartridgeController.CartridgeFileName = "";
        CartridgeController.CartridgeTitle = "";
        CartridgeController.CartridgeVersion = 0;
        CartridgeController.CartridgeRevision = 0;
        
        // tell GPU to release all cartridge textures
        GPU.RemoveCartridgeTextures();
        
        // tell SPU to release all cartridge sounds
        for( int i = 0; i < Constants::SPUMaximumCartridgeSounds; i++ )
          SPU.UnloadSound( SPU.CartridgeSounds[ i ] );
        
        SPU.LoadedCartridgeSounds = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::HasCartridge()
    {
        return (CartridgeController.MemorySize != 0);
    }
    
    // -----------------------------------------------------------------------------
    
    string V32Console::GetCartridgeFileName()
    {
        return CartridgeController.CartridgeFileName;
    }
    
    // -----------------------------------------------------------------------------
    
    string V32Console::GetCartridgeTitle()
    {
        return CartridgeController.CartridgeTitle;
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: MEMORY CARD MANAGEMENT
    // =============================================================================
    
    
    void V32Console::CreateMemoryCard( const std::string& FilePath )
    {
        Callbacks::LogLine( "Creating memory card" );
        Callbacks::LogLine( "File path: \"" + FilePath + "\"" );
        
        // open the file
        ofstream OutputFile;
        
        // on windows convert path from UTF-8 to UTF-16
        #if defined(__WIN32__)
          wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
          wstring FilePathUTF16 = converter.from_bytes(FilePath);
          OutputFile.open( FilePathUTF16.c_str(), ios_base::binary | ios::trunc );
        #else
          OutputFile.open( FilePath, ios_base::binary | ios::trunc );
        #endif
        
        if( OutputFile.fail() )
          Callbacks::ThrowException( "Cannot create memory card file" );
        
        // save the signature
        WriteSignature( OutputFile, MemoryCardFileFormat::Signature );
        
        // now save all empty contents
        vector< V32Word > EmptyWords;
        EmptyWords.resize( Constants::MemoryCardSize );
        OutputFile.write( (char*)(&EmptyWords[ 0 ]), Constants::MemoryCardSize * 4 );
        
        // close the file
        OutputFile.close();
        Callbacks::LogLine( "Finished creating memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::LoadMemoryCard( const std::string& FilePath )
    {
        Callbacks::LogLine( "Loading memory card" );
        Callbacks::LogLine( "File path: \"" + FilePath + "\"" );
    
        // unload any previous card
        UnloadMemoryCard();
        
        // open the file for random access
        fstream& InputFile = MemoryCardController.LinkedFile;
        
        // on windows convert path from UTF-8 to UTF-16
        #if defined(__WIN32__)
          wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
          wstring FilePathUTF16 = converter.from_bytes(FilePath);
          InputFile.open( FilePathUTF16.c_str(), ios_base::in | ios_base::out | ios::binary | ios::ate );
        #else
          InputFile.open( FilePath, ios_base::in | ios_base::out | ios::binary | ios::ate );
        #endif
        
        if( InputFile.fail() )
          Callbacks::ThrowException( "Cannot open memory card file" );
        
        // check file size coherency
        int NumberOfBytes = InputFile.tellg();
        int ExpectedBytes = 8 + Constants::MemoryCardSize * 4;
        
        if( NumberOfBytes != ExpectedBytes )
        {
            InputFile.close();
            Callbacks::ThrowException( "Invalid memory card: File does not match the size of a Vircon memory card" );
        }
        
        // read and check signature
        InputFile.seekg( 0, ios_base::beg );
        char FileSignature[ 8 ];
        InputFile.read( FileSignature, 8 );
        
        if( !CheckSignature( FileSignature, MemoryCardFileFormat::Signature ) )
          Callbacks::ThrowException( "Memory card file does not have a valid signature" );
        
        // connect the memory
        MemoryCardController.Connect( Constants::MemoryCardSize );
        
        // now load the whole memory card contents
        InputFile.read( (char*)(&MemoryCardController.Memory[ 0 ]), Constants::MemoryCardSize * 4 );
        
        // do NOT close the file! leave it open until
        // card is unloaded or emulation is stopped,
        // so that it can be saved if card is modified
        
        // save the file name
        MemoryCardController.CardFileName = GetPathFileName( FilePath );
        Callbacks::LogLine( "Finished loading memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::UnloadMemoryCard()
    {
        // do nothing if a card is not loaded
        if( !HasMemoryCard() ) return;
        Callbacks::LogLine( "Unloading memory card" );
        
        // save the card if it was modified
        if( MemoryCardController.PendingSave )
          SaveMemoryCard();
        
        // remove the card memory
        MemoryCardController.Disconnect();
        
        // close the open file
        MemoryCardController.LinkedFile.close();
        Callbacks::LogLine( "Finished unloading memory card" );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::SaveMemoryCard()
    {
        // do nothing if a card is not loaded
        if( !HasMemoryCard() ) return;
        
        // check the file
        fstream& OutputFile = MemoryCardController.LinkedFile;
        
        if( !OutputFile.is_open() || OutputFile.fail() )
          Callbacks::ThrowException( "Cannot save memory card file" );
        
        // save the signature
        OutputFile.seekp( ios_base::beg );
        WriteSignature( OutputFile, MemoryCardFileFormat::Signature );
        
        // now save all contents
        OutputFile.write( (char*)(&MemoryCardController.Memory[ 0 ]), Constants::MemoryCardSize * 4 );
        MemoryCardController.PendingSave = false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::HasMemoryCard()
    {
        return (MemoryCardController.MemorySize != 0);
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::WasMemoryCardModified()
    {
        return MemoryCardController.PendingSave;
    }
    
    // -----------------------------------------------------------------------------
    
    string V32Console::GetMemoryCardFileName()
    {
        return MemoryCardController.CardFileName;
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: GAMEPAD MANAGEMENT
    // =============================================================================
    
    
    void V32Console::SetGamepadConnection( int GamepadPort, bool Connected )
    {
        // this function is just an external interface:
        // just pass the call to the gamepad controller
        GamepadController.SetGamepadConnection( GamepadPort, Connected );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::SetGamepadControl( int GamepadPort, GamepadControls Control, bool Pressed )  
    {
        // this function is just an external interface:
        // just pass the call to the gamepad controller
        GamepadController.SetGamepadControl( GamepadPort, Control, Pressed );
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32Console::HasGamepad( int GamepadPort )
    {
        if( GamepadPort < 0 || GamepadPort >= Constants::GamepadPorts )
          return false;
        
        return GamepadController.RealTimeGamepadStates[ GamepadPort ].Connected;
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: TIMER MANAGEMENT
    // =============================================================================
    
    
    void V32Console::SetCurrentDate( int Year, int DaysWithinYear )
    {
        // input range checks
        Clamp( Year, 0, 32767 );
        Clamp( DaysWithinYear, 0, 365 );
        
        Timer.CurrentDate = (Year << 16) | DaysWithinYear;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32Console::SetCurrentTime( int Hours, int Minutes, int Seconds )
    {
        // input range checks
        Clamp( Hours,   0, 23 );
        Clamp( Minutes, 0, 59 );
        Clamp( Seconds, 0, 59 );
        
        Timer.CurrentTime = (Hours * 3600) + (Minutes * 60) + Seconds;
    }
    
    
    // =============================================================================
    //      V32 CONSOLE: SOUND OUTPUT MANAGEMENT
    // =============================================================================
    
    
    void V32Console::GetFrameSoundOutput( SPUOutputBuffer& OutputBuffer )
    {
        // for safety, make a copy of the sound buffer
        // instead of providing access to the original
        memcpy( &OutputBuffer, &SPU.OutputBuffer, sizeof(SPU.OutputBuffer) );
    }
}
