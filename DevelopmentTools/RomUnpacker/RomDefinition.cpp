// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/Definitions.hpp"
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/FileSignatures.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "RomDefinition.hpp"
    
    // include C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      ROM DEFINITION: SECONDARY FUNCTIONS
// =============================================================================


void RomDefinition::ExtractBinary( BinaryFileFormat::Header& BinaryHeader, vector< V32Word >& BinaryWords )
{
    // determine file path
    string BinaryFilePath = BaseFolder + PathSeparator + RomFileName + ".vbin";
    
    // open output file as binary
    ofstream BinaryFile;
    BinaryFile.open( BinaryFilePath, ios_base::binary );
    
    // write all data into the file
    BinaryFile.write( (char*)(&BinaryHeader), sizeof( BinaryFileFormat::Header ) );
    BinaryFile.write( (char*)(&BinaryWords[ 0 ]), 4 * BinaryWords.size() );
    BinaryFile.close();
}

// -----------------------------------------------------------------------------

void RomDefinition::ExtractTexture( TextureFileFormat::Header& TextureHeader, vector< V32Word >& TexturePixels )
{
    // determine current file path
    string TextureFilePath = BaseFolder + PathSeparator + "textures"
    + PathSeparator + "texture" + to_string( ExtractedTextures ) + ".vtex";
    
    // open output file as binary
    ofstream TextureFile;
    TextureFile.open( TextureFilePath, ios_base::binary );
    
    // write all data into the file
    TextureFile.write( (char*)(&TextureHeader), sizeof( TextureFileFormat::Header ) );
    TextureFile.write( (char*)(&TexturePixels[ 0 ]), 4 * TexturePixels.size() );
    TextureFile.close();
    
    // count the textures
    ExtractedTextures++;
}

// -----------------------------------------------------------------------------

void RomDefinition::ExtractSound( SoundFileFormat::Header& SoundHeader, vector< V32Word >& SoundSamples )
{
    // determine current file path
    string SoundFilePath = BaseFolder + PathSeparator + "sounds"
    + PathSeparator + "sound" + to_string( ExtractedSounds ) + ".vsnd";
    
    // open output file as binary
    ofstream SoundFile;
    SoundFile.open( SoundFilePath, ios_base::binary );
    
    // write all data into the file
    SoundFile.write( (char*)(&SoundHeader), sizeof( SoundFileFormat::Header ) );
    SoundFile.write( (char*)(&SoundSamples[ 0 ]), 4 * SoundSamples.size() );
    SoundFile.close();
    
    // count the sounds
    ExtractedSounds++;
}

// -----------------------------------------------------------------------------

void RomDefinition::CreateDefinitionXML()
{
    string XMLFilePath = BaseFolder + PathSeparator + RomFileName + ".xml";
    
    // open output file as text
    ofstream XMLFile;
    XMLFile.open( XMLFilePath );
    
    if( !XMLFile.good() )
      throw runtime_error( "cannot create XML file" );
    
    // write XML start and Vircon32 version
    XMLFile << "<rom-definition version=\"";
    XMLFile << VirconVersion << "." << VirconRevision << "\">" << endl;
    
    // write basic ROM metadata
    XMLFile << "    <rom type=\"" << (IsBios? "bios" : "cartridge");
    XMLFile << "\" title=\"" << Title;
    XMLFile << "\" version=\"" << ROMVersion << "." << ROMRevision << "\" />" << endl;

    // write ROM binary
    XMLFile << "    <binary path=\"" << RomFileName << ".vbin\" />" << endl;
    
    // write ROM textures
    XMLFile << "    <textures>" << endl;
    
    for( int i = 0; i < ExtractedTextures; i++ )
      XMLFile << "        <texture path=\"textures/texture" << i << ".vtex\" />" << endl;
    
    XMLFile << "    </textures>" << endl;
    
    // write ROM sounds
    XMLFile << "    <sounds>" << endl;
    
    for( int i = 0; i < ExtractedSounds; i++ )
      XMLFile << "        <sound path=\"sounds/sound" << i << ".vsnd\" />" << endl;
    
    XMLFile << "    </sounds>" << endl;
    
    // write XML end
    XMLFile << "</rom-definition>" << endl;
    XMLFile.close();
}

// -----------------------------------------------------------------------------

void RomDefinition::CreateMakeBAT()
{
    string BATFilePath = BaseFolder + PathSeparator + "Make.bat";
    
    // open output file as text
    ofstream BATFile;
    BATFile.open( BATFilePath );
    
    if( !BATFile.good() )
      throw runtime_error( "cannot create BAT file" );
    
    // write initial section
    BATFile << "@echo off" << endl;
    BATFile << endl;
    BATFile << "REM prepare the script to be run from another directory" << endl;
    BATFile << "pushd %~dp0" << endl;
    BATFile << endl;
    BATFile << "REM create bin folder if non exiting, since" << endl;
    BATFile << "REM the development tools will not create it themselves" << endl;
    BATFile << "if not exist bin mkdir bin" << endl;
    BATFile << endl;
    
    // write packer section
    BATFile << "echo." << endl;
    BATFile << "echo Pack the ROM" << endl;
    BATFile << "echo --------------------------" << endl;
    BATFile << "packrom \"" << RomFileName << ".xml\" -o \"bin\\" << RomFileName << ".v32\" || goto :failed" << endl;
    BATFile << "goto :succeeded" << endl;
    BATFile << endl;
    
    // write final section
    BATFile << ":failed" << endl;
    BATFile << "popd" << endl;
    BATFile << "echo." << endl;
    BATFile << "echo BUILD FAILED" << endl;
    BATFile << "exit /b %errorlevel%" << endl;
    BATFile << endl;
    BATFile << ":succeeded" << endl;
    BATFile << "popd" << endl;
    BATFile << "echo." << endl;
    BATFile << "echo BUILD SUCCESSFUL" << endl;
    BATFile << "exit /b" << endl;
    BATFile << endl;
    BATFile << "@echo on" << endl;
    
    BATFile.close();
}

// -----------------------------------------------------------------------------

void RomDefinition::CreateMakeSH()
{
    string SHFilePath = BaseFolder + PathSeparator + "Make.sh";
    
    // open output file as text
    ofstream SHFile;
    SHFile.open( SHFilePath );
    
    if( !SHFile.good() )
      throw runtime_error( "cannot create SH file" );
    
    // write initial section
    SHFile << "#!/bin/bash" << endl;
    SHFile << endl;
    SHFile << "# create bin folders if non existing, since the" << endl;
    SHFile << "# development tools will not create it themselves" << endl;
    SHFile << "mkdir -p bin" << endl;
    SHFile << "# define an abort function to call on error" << endl;
    SHFile << "abort_build()" << endl;
    SHFile << endl;
    SHFile << "    echo" << endl;
    SHFile << "    echo BUILD FAILED" << endl;
    SHFile << "    exit 1" << endl;
    SHFile << "}" << endl;
    SHFile << endl;
    
    // write packer section
    SHFile << "echo" << endl;
    SHFile << "echo Pack the ROM" << endl;
    SHFile << "echo --------------------------" << endl;
    SHFile << "packrom \"" << RomFileName << ".xml\" -o \"bin/" << RomFileName << ".v32\" || abort_build" << endl;
    SHFile << endl;
    
    // write final section
    SHFile << "echo" << endl;
    SHFile << "echo BUILD SUCCESSFUL" << endl;
    
    SHFile.close();
}


// =============================================================================
//      ROM DEFINITION: MAIN METHODS
// =============================================================================


void RomDefinition::UnpackROM( const std::string& InputPath, const std::string& OutputPath )
{
    // store base folder path
    RomFileName = GetPathFileName( InputPath );
    RomFileName = GetFileWithoutExtension( RomFileName );
    
    BaseFolder = OutputPath;
    
    // open the ROM file
    ifstream InputFile;
    InputFile.open( InputPath, ios_base::binary | ios_base::ate );
    
    if( !InputFile.good() )
      throw runtime_error( "cannot open input file" );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Load global information
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = InputFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "incorrect V32 file format (file size must be a multiple of 4)" );
    
    // ensure that we can at least load the file header
    if( FileBytes < sizeof(ROMFileFormat::Header) )
      throw runtime_error( "incorrect V32 file format (file is too small)" );
    
    // now we can safely read the global header
    InputFile.seekg( 0, ios_base::beg );
    ROMFileFormat::Header ROMHeader;
    InputFile.read( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
    
    // check if the ROM is actually a BIOS
    if( CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
      IsBios = false;
    
    else if( CheckSignature( ROMHeader.Signature, ROMFileFormat::BiosSignature ) )
      IsBios = true;
    
    else throw runtime_error( "signature for input V32 ROM is not recognized as either cartridge or bios" );
    
    // now check the actual cartridge signature
    if( !CheckSignature( ROMHeader.Signature, ROMFileFormat::CartridgeSignature ) )
      throw runtime_error( "incorrect V32 file format (file does not have a valid signature)" );
    
    // check current Vircon version
    if( ROMHeader.VirconVersion  > (unsigned)Constants::VirconVersion
    ||  ROMHeader.VirconRevision > (unsigned)Constants::VirconRevision )
      throw runtime_error( "this cartridge was made for a more recent version of Vircon32. Please use an updated emulator" );
    
    // read the main header and store info
    ROMHeader.Title[ 63 ] = 0;
    Title = ROMHeader.Title;
    VirconVersion = ROMHeader.VirconVersion;
    VirconRevision = ROMHeader.VirconRevision;
    ROMVersion = ROMHeader.ROMVersion;
    ROMRevision = ROMHeader.ROMRevision;
    
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
    if( ROMHeader.ProgramROMLocation.StartOffset != sizeof(ROMFileFormat::Header) )
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
    // STEP 3: Extract program binary
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // load a binary file signature
    BinaryFileFormat::Header BinaryHeader;
    InputFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
    
    // check signature for embedded binary
    if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
      throw runtime_error( "Cartridge binary does not have a valid signature" );
    
    // check program rom size limitations
    if( !IsBetween( BinaryHeader.NumberOfWords, 1, Constants::MaximumCartridgeProgramROM ) )
      throw runtime_error( "Cartridge program ROM does not have a correct size (from 1 word up to 128M words)" );
    
    // load the binary words
    vector< V32Word > LoadedBinary;
    LoadedBinary.resize( BinaryHeader.NumberOfWords );
    InputFile.read( (char*)(&LoadedBinary[ 0 ]), BinaryHeader.NumberOfWords * 4 );
    
    // extract contents of program binary
    ExtractBinary( BinaryHeader, LoadedBinary );
    LoadedBinary.clear();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Extract texture binaries
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // create the textures folder if needed
    if( ROMHeader.NumberOfTextures > 0 )
    {
        string TexturesFolderPath = BaseFolder + PathSeparator + "textures";
        
        if( !DirectoryExists( TexturesFolderPath ) )
          if( !CreateDirectory( TexturesFolderPath ) )
            throw runtime_error( "Cannot create textures folder" );
    }
    
    // keep extracting textures
    ExtractedTextures = 0;
    
    for( unsigned i = 0; i < ROMHeader.NumberOfTextures; i++ )
    {
        // load a texture file signature
        TextureFileFormat::Header TextureHeader;
        InputFile.read( (char*)(&TextureHeader), sizeof(TextureFileFormat::Header) );
        
        // check signature for embedded texture
        if( !CheckSignature( TextureHeader.Signature, TextureFileFormat::Signature ) )
          throw runtime_error( "Cartridge texture does not have a valid signature" );
        
        // check texture size limitations
        if( !IsBetween( TextureHeader.TextureWidth , 1, 1024 )
        ||  !IsBetween( TextureHeader.TextureHeight, 1, 1024 ) )
          throw runtime_error( "Cartridge texture does not have correct dimensions (1x1 up to 1024x1024 pixels)" );
        
        // load the texture pixels
        vector< V32Word > LoadedTexture;
        int NumberOfPixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
        LoadedTexture.resize( NumberOfPixels );
        InputFile.read( (char*)(&LoadedTexture[ 0 ]), NumberOfPixels * 4 );
        
        // extract contents of texture binary
        ExtractTexture( TextureHeader, LoadedTexture );
        LoadedTexture.clear();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 5: Extract sound binaries
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // create the sounds folder if needed
    if( ROMHeader.NumberOfSounds > 0 )
    {
        string SoundsFolderPath = BaseFolder + PathSeparator + "sounds";
        
        if( !DirectoryExists( SoundsFolderPath ) )
          if( !CreateDirectory( SoundsFolderPath ) )
            throw runtime_error( "Cannot create sounds folder" );
    }
    
    // keep extracting sounds
    ExtractedSounds = 0;
    uint32_t TotalSPUSamples = 0;
    
    for( unsigned i = 0; i < ROMHeader.NumberOfSounds; i++ )
    {
        // load a sound file signature
        SoundFileFormat::Header SoundHeader;
        InputFile.read( (char*)(&SoundHeader), sizeof(SoundFileFormat::Header) );
        
        // check signature for embedded sound
        if( !CheckSignature( SoundHeader.Signature, SoundFileFormat::Signature ) )
          throw runtime_error( "Cartridge sound does not have a valid signature" );
        
        // check length limitations for this sound
        if( !IsBetween( SoundHeader.SoundSamples, 1, Constants::SPUMaximumCartridgeSamples ) )
          throw runtime_error( "Cartridge sound does not have correct length (1 up to 256M samples)" );
        
        // check length limitations for the whole SPU
        TotalSPUSamples += SoundHeader.SoundSamples;
        
        if( TotalSPUSamples > (uint32_t)Constants::SPUMaximumCartridgeSamples )
          throw runtime_error( "Cartridge sounds contain too many total samples (Vircon SPU only allows up to 256M total samples)" );
        
        // load the sound samples
        vector< V32Word > LoadedSound;
        LoadedSound.resize( SoundHeader.SoundSamples );
        InputFile.read( (char*)(&LoadedSound[ 0 ]), SoundHeader.SoundSamples * 4 );
        
        // extract contents of sound binary
        ExtractSound( SoundHeader, LoadedSound );
        LoadedSound.clear();
    }
    
    // we can now close the input ROM file
    InputFile.close();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 6: Create XML definition and make scripts
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // create the XML file for rom definition
    CreateDefinitionXML();
    
    // create make scripts
    CreateMakeBAT();
    CreateMakeSH();
}
