// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
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
    
    // include TinyXML2 headers
    #include <tinyxml2.h>       // [ TinyXML2 ] Main header
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
    using namespace tinyxml2;
// *****************************************************************************


// =============================================================================
//      XML HELPER FUNCTIONS
// =============================================================================


// automation for child elements in XML
XMLElement* GetRequiredElement( XMLElement* Parent, const string& ChildName )
{
    if( !Parent )
      throw runtime_error( "Parent element NULL" );
    
    XMLElement* Child = Parent->FirstChildElement( ChildName.c_str() );
    
    if( !Child )
      throw runtime_error( string("Cannot find element <") + ChildName + "> inside <" + Parent->Name() + ">" );
    
    return Child;
}

// -----------------------------------------------------------------------------

// automation for string attributes in XML
string GetRequiredStringAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      throw runtime_error( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      throw runtime_error( string("Cannot find attribute '") + AtributeName + "' inside <" + Element->Name() + ">" );
    
    return Attribute->Value();
}

// -----------------------------------------------------------------------------

// automation for integer attributes in XML
int GetRequiredIntegerAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      throw runtime_error( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      throw runtime_error( string("Cannot find attribute '") + AtributeName + "' inside <" + Element->Name() + ">" );
    
    // attempt integer conversion
    int Number = 0;
    XMLError ErrorCode = Element->QueryIntAttribute( AtributeName.c_str(), &Number );
    
    if( ErrorCode != XML_SUCCESS )
      throw runtime_error( string("Attribute '") + AtributeName + "' inside <" + Element->Name() + "> must be an integer number" );
    
    return Number;
}


// =============================================================================
//      ROM AUXILIARY FUNCTIONS
// =============================================================================


void CopySignature( char* Destination, const char* Value )
{
    for( int i = 0; i < 8; i++ )
      Destination[i] = Value[i];
}

// -----------------------------------------------------------------------------

void ParseVersionString( const string& VersionText, uint32_t& Version, uint32_t& Revision )
{
    // check that there are only digits and dots
    for( char c: VersionText )
      if( !isdigit(c) && (c != '.') )
        throw runtime_error( "ROM version string contains invalid characters" );
    
    // locate the dot
    size_t DotPosition = VersionText.find( '.' );
    
    if( DotPosition == string::npos )
      throw runtime_error( "ROM version string should contain a dot as separator" );
    
    // there has to be at least 1 digit at each side of the dot
    if( DotPosition == 0 || (VersionText.length()-DotPosition) < 2 )
      throw runtime_error( "ROM version string should contain 2 numbers" );
    
    // parse each part as an integer
    string FirstNumber = VersionText.substr( 0, DotPosition );
    Version = stoul( FirstNumber );
    
    // parse second part as integer
    string SecondNumber = VersionText.substr( DotPosition+1 );
    Revision = stoul( SecondNumber );
}


// =============================================================================
//      ROM DEFINITION: SECONDARY FUNCTIONS
// =============================================================================


void RomDefinition::ProcessBinary( string& BinaryPath, vector< uint32_t >& ProgramROM )
{
    // correction for path folder (only if relative)
    if( BinaryPath.find(':') == string::npos )
      BinaryPath = BaseFolder + PathSeparator + BinaryPath;
    
    // open the file
    ifstream BinaryFile;
    BinaryFile.open( BinaryPath, ios_base::binary | ios_base::ate );
    
    if( !BinaryFile.good() )
      throw runtime_error( "cannot open binary file" );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = BinaryFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "incorrect VBIN format (file size must be a multiple of 4)" );
    
    // file size should be at least 4 dwords
    // (i.e. header + 1 instruction)
    unsigned FileWords = FileBytes / 4;
    
    if( FileWords < 4 )
      throw runtime_error( "incorrect VBIN format (file is too small)" );
    
    // read program header
    BinaryFileFormat::Header BinaryHeader;
    BinaryFile.seekg( 0, ios_base::beg );
    BinaryFile.read( (char*)(&BinaryHeader), sizeof(BinaryFileFormat::Header) );
    
    // check signature
    if( !CheckSignature( BinaryHeader.Signature, BinaryFileFormat::Signature ) )
      throw runtime_error( "incorrect VBIN format (file size does contain a VBIN signature)" );
    
    // file size must match the indicated program size
    if( FileWords != BinaryHeader.NumberOfWords + 3 )
      throw runtime_error( "incorrect VBIN format (file size does not match indicated program size)" );
    
    // now we can just copy the whole file to program rom,
    // since the header is also included in the final rom
    ProgramROM.resize( FileWords );
    BinaryFile.seekg( 0, ios_base::beg );
    BinaryFile.read( (char*)(&ProgramROM[0]), FileBytes );
    
    // close the file
    BinaryFile.close();
}

// -----------------------------------------------------------------------------

void RomDefinition::ProcessTexture( string& TexturePath, vector< uint32_t >& TextureROM )
{
    // correction for path folder (only if relative)
    if( TexturePath.find(':') == string::npos )
      TexturePath = BaseFolder + PathSeparator + TexturePath;
    
    // open the file
    ifstream TextureFile;
    TextureFile.open( TexturePath, ios_base::binary | ios_base::ate );
    
    if( !TextureFile.good() )
      throw runtime_error( "cannot open texture file" );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = TextureFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "incorrect VTEX format (file size must be a multiple of 4)" );
    
    // file size should be at least 5 dwords
    // (i.e. header + 1 pixel)
    unsigned FileWords = FileBytes / 4;
    
    if( FileWords < 5 )
      throw runtime_error( "incorrect VTEX format (file is too small)" );
    
    // read texture header
    TextureFileFormat::Header TextureHeader;
    TextureFile.seekg( 0, ios_base::beg );
    TextureFile.read( (char*)(&TextureHeader), sizeof(TextureFileFormat::Header) );
    
    // check signature
    if( !CheckSignature( TextureHeader.Signature, TextureFileFormat::Signature ) )
      throw runtime_error( "incorrect VTEX format (file size does contain a VTEX signature)" );
    
    // file size must match the indicated dimensions
    uint32_t TexturePixels = TextureHeader.TextureWidth * TextureHeader.TextureHeight;
    
    if( FileWords != TexturePixels + 4 )
      throw runtime_error( "incorrect VTEX format (file size does not match image dimensions)" );
    
    // check texture dimension limits
    if( TextureHeader.TextureWidth > 1024u || TextureHeader.TextureHeight > 1024u )
      throw runtime_error( "texture size is larger than allowed by Vircon32 GPU" );
    
    // now we can just copy the whole file to texture rom,
    // since the header is also included in the final rom
    TextureROM.resize( FileWords );
    TextureFile.seekg( 0, ios_base::beg );
    TextureFile.read( (char*)(&TextureROM[0]), FileBytes );
    
    // close the file
    TextureFile.close();
}

// -----------------------------------------------------------------------------

void RomDefinition::ProcessSound( string& SoundPath, vector< uint32_t >& SoundROM )
{
    // correction for path folder (only if relative)
    if( SoundPath.find(':') == string::npos )
      SoundPath = BaseFolder + PathSeparator + SoundPath;
    
    // open the file
    ifstream SoundFile;
    SoundFile.open( SoundPath, ios_base::binary | ios_base::ate );
    
    if( !SoundFile.good() )
      throw runtime_error( "cannot open sound file" );
    
    // get size and ensure it is a multiple of 4
    // (otherwise file contents are wrong)
    unsigned FileBytes = SoundFile.tellg();
    
    if( (FileBytes % 4) != 0 )
      throw runtime_error( "incorrect VSND format (file size must be a multiple of 4)" );
    
    // file size should be at least 4 dwords
    // (i.e. header + 1 sample)
    unsigned FileWords = FileBytes / 4;
    
    if( FileWords < 4 )
      throw runtime_error( "incorrect VSND format (file is too small)" );
      
    // read sound header
    SoundFileFormat::Header SoundHeader;
    SoundFile.seekg( 0, ios_base::beg );
    SoundFile.read( (char*)(&SoundHeader), sizeof(SoundFileFormat::Header) );
    
    // check signature
    if( !CheckSignature( SoundHeader.Signature, SoundFileFormat::Signature ) )
      throw runtime_error( "incorrect VSND format (file size does contain a VSND signature)" );
    
    // file size must match the indicated sound length
    if( FileWords != SoundHeader.SoundSamples + 3 )
      throw runtime_error( "incorrect VSND format (file size does not match indicated sound size)" );
    
    // check sound length limit
    if( SoundHeader.SoundSamples > (int)Constants::SPUMaximumCartridgeSamples )
      throw runtime_error( "sound size is larger than allowed by Vircon32 SPU" );
    
    // now we can just copy the whole file to sound rom,
    // since the header is also included in the final rom
    SoundROM.resize( FileWords );
    SoundFile.seekg( 0, ios_base::beg );
    SoundFile.read( (char*)(&SoundROM[0]), FileBytes );
    
    // close the file
    SoundFile.close();
}


// =============================================================================
//      ROM DEFINITION: MAIN METHODS
// =============================================================================


void RomDefinition::LoadXML( const string& InputPath )
{
    // first, check if the input file exists
    if( !FileExists( InputPath ) )
      throw runtime_error( string("cannot open input file \"") + InputPath + "\"" );
    
    // load XML file
    XMLDocument Loaded;
    XMLError ErrorCode = Loaded.LoadFile( InputPath.c_str() );
    
    if( ErrorCode != XML_SUCCESS )
      throw runtime_error( "Cannot read XML from file path " + InputPath );
    
    // obtain XML root
    XMLElement* Root = Loaded.FirstChildElement( "rom-definition" );
    
    if( !Root )
      throw runtime_error( "Cannot find <rom-definition> root element" );
    
    // read and convert vircon version and revision
    string VirconVersionText = GetRequiredStringAttribute( Root, "version" );
    uint32_t VirconVersion, VirconRevision;
    ParseVersionString( VirconVersionText, VirconVersion, VirconRevision );
    
    // check against current version
    if( VirconVersion  > (unsigned)Constants::VirconVersion 
    ||  VirconRevision > (unsigned)Constants::VirconRevision )
      throw runtime_error( "this ROM definition was made for a more recent version of Vircon. Please use the corresponding updated tools" );
    
    // before loading resources, verify that all main elements exist
    XMLElement* Rom      = GetRequiredElement( Root, "rom"      );
    XMLElement* Binary   = GetRequiredElement( Root, "binary"   );
    XMLElement* Textures = GetRequiredElement( Root, "textures" );
    XMLElement* Sounds   = GetRequiredElement( Root, "sounds"   );
    
    // read rom title and check length
    Title = GetRequiredStringAttribute( Rom, "title" );
    
    if( Title.length() > 63 )
      throw runtime_error( "ROM title is longer than the maximum of 63 characters" );
    
    // read rom type
    string ROMType = GetRequiredStringAttribute( Rom, "type" );
    
    if( ToLowerCase( ROMType ) == "bios" )
      IsBios = true;
    
    else if( ToLowerCase( ROMType ) == "cartridge" )
      IsBios = false;
    
    else
      throw runtime_error( "ROM type is invalid (must be either 'cartridge' or 'bios')" );
    
    // read and convert rom version and revision
    string RomVersionText = GetRequiredStringAttribute( Rom, "version" );
    ParseVersionString( RomVersionText, Version, Revision );
    
    // read path for program rom
    BinaryPath = GetRequiredStringAttribute( Binary, "path" );
    
    // read all texture paths
    // (optional: there could be none)
    XMLElement* CurrentTexture = Textures->FirstChildElement( "texture" );
    
    while( CurrentTexture != nullptr )
    {
        TexturePaths.push_back( GetRequiredStringAttribute( CurrentTexture, "path" ) );
        CurrentTexture = CurrentTexture->NextSiblingElement( "texture" );
    }
    
    // read all sound paths
    // (optional: there could be none)
    XMLElement* CurrentSound = Sounds->FirstChildElement( "sound" );
    
    while( CurrentSound != nullptr )
    {
        SoundPaths.push_back( GetRequiredStringAttribute( CurrentSound, "path" ) );
        CurrentSound = CurrentSound->NextSiblingElement( "sound" );
    }
    
    // check that bios roms have 1 texture and 1 sound
    if( IsBios )
    {
        if( TexturePaths.size() != 1u || SoundPaths.size() != 1u )
          throw runtime_error( "ROM definitions for BIOS must contain exactly 1 texture and 1 sound" );
    }
    
    // check that cartridge maximums are not exceeded
    else
    {
        if( (int)TexturePaths.size() > Constants::GPUMaximumCartridgeTextures )
          throw runtime_error( "ROM definition contains more textures than allowed by Vircon32 GPU" );
        
        if( (int)SoundPaths.size() > Constants::SPUMaximumCartridgeSounds )
          throw runtime_error( "ROM definition contains more sounds than allowed by Vircon32 SPU" );
    }
}

// -----------------------------------------------------------------------------

void RomDefinition::PackROM( const string& OutputPath )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Load program ROM
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // process the program file
    vector< uint32_t > ProgramROM;
    
    try
    {
        ProcessBinary( BinaryPath, ProgramROM );
    }
    
    // do this to always report the specific file on an error
    catch( const exception& e )
    {
        throw runtime_error( string("in binary file \"") + BinaryPath + "\" " + e.what() );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Build video ROM from all textures
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // process each texture file
    vector< vector< uint32_t > > VideoROM;
    
    for( string TexturePath: TexturePaths )
    {
        try
        {
            VideoROM.emplace_back();
            ProcessTexture( TexturePath, VideoROM.back() );
        }
        
        // do this to always report the specific file on an error
        catch( const exception& e )
        {
            throw runtime_error( string("in texture file \"") + TexturePath + "\" " + e.what() );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 3: Build audio ROM from all sounds
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // process each sound file
    vector< vector< uint32_t > > AudioROM;
    
    for( string SoundPath: SoundPaths )
    {
        try
        {
            AudioROM.emplace_back();
            ProcessSound( SoundPath, AudioROM.back() );
        }
        
        // do this to always report the specific file on an error
        catch( const exception& e )
        {
            throw runtime_error( string("in sound file \"") + SoundPath + "\" " + e.what() );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Create the ROM file header
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // make ROM header initially empty to
    // ensure zeroes at all unused bytes
    ROMFileFormat::Header ROMHeader;
    memset( &ROMHeader, 0, sizeof(ROMFileFormat::Header) );
    
    // fill in Vircon metadata
    CopySignature( ROMHeader.Signature, IsBios? ROMFileFormat::BiosSignature : ROMFileFormat::CartridgeSignature );
    ROMHeader.VirconVersion  = Constants::VirconVersion;
    ROMHeader.VirconRevision = Constants::VirconRevision;
    
    // fill in ROM metadata
    strncpy( ROMHeader.Title, Title.c_str(), 63 );
    ROMHeader.ROMVersion = Version;
    ROMHeader.ROMRevision = Revision;
    
    // count the number of assets
    ROMHeader.NumberOfTextures = VideoROM.size();
    ROMHeader.NumberOfSounds = AudioROM.size();
    
    // calculate bytes of program ROM in the file
    ROMHeader.ProgramROMLocation.StartOffset = sizeof(ROMFileFormat::Header);
    ROMHeader.ProgramROMLocation.Length = ProgramROM.size() * 4;
    
    // calculate the total size in bytes of video ROM
    // (in the file! not in console GPU)
    ROMHeader.VideoROMLocation.StartOffset = ROMHeader.ProgramROMLocation.StartOffset + ROMHeader.ProgramROMLocation.Length;
    ROMHeader.VideoROMLocation.Length = 0;
    
    for( vector< uint32_t > TextureROM: VideoROM )
      ROMHeader.VideoROMLocation.Length += 4 * TextureROM.size();
    
    // calculate the total size in bytes of audio ROM
    // (in the file! not in console SPU)
    ROMHeader.AudioROMLocation.StartOffset = ROMHeader.VideoROMLocation.StartOffset + ROMHeader.VideoROMLocation.Length;
    ROMHeader.AudioROMLocation.Length = 0;
    
    for( vector< uint32_t > SoundROM: AudioROM )
      ROMHeader.AudioROMLocation.Length += 4 * SoundROM.size();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 4: Build the output file from all partial ROMs
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // open the output file
    ofstream OutputFile;
    OutputFile.open( OutputPath, ios_base::binary );
    
    if( !OutputFile.good() )
      throw runtime_error( string("cannot open output file \"") + OutputPath + "\"" );
    
    // write global header to file
    OutputFile.write( (char*)(&ROMHeader), sizeof(ROMFileFormat::Header) );
    
    // write program ROM
    OutputFile.write( (char*)(&ProgramROM[ 0 ]), ProgramROM.size() * 4 );
    
    // write video ROM
    for( vector< uint32_t >& TextureROM: VideoROM )
      OutputFile.write( (char*)(&TextureROM[ 0 ]), TextureROM.size() * 4 );
    
    // write audio ROM
    for( vector< uint32_t >& SoundROM: AudioROM )
      OutputFile.write( (char*)(&SoundROM[ 0 ]), SoundROM.size() * 4 );
    
    // close the output file
    OutputFile.close();
}
