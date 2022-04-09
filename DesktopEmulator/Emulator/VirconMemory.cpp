// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "VirconMemory.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: VIRCON RAM
// =============================================================================


VirconRAM::VirconRAM()
{
    MemorySize = 0;
}

// -----------------------------------------------------------------------------

void VirconRAM::Connect( uint32_t NumberOfWords )
{
    // disconnect previous memory
    Disconnect();
    
    // connect new one
    Memory.resize( NumberOfWords );
    MemorySize = NumberOfWords;
    
    // initially, set to zeroes
    ClearContents();
}

// -----------------------------------------------------------------------------

void VirconRAM::Disconnect()
{
    Memory.clear();
    MemorySize = 0;
}

// -----------------------------------------------------------------------------

void VirconRAM::SaveContents( const string& FilePath )
{
    // open the file
    LOG( "Saving RAM file \"" << FilePath << "\"" );
    
    ofstream OutputFile;
    OutputFile.open( FilePath, ios::binary );
    
    if( OutputFile.fail() )
      THROW( "Cannot open RAM file" );
    
    // save all contents
    OutputFile.write( (char*)(&Memory[0]), MemorySize * 4 );
    
    // close the file
    OutputFile.close();
}

// -----------------------------------------------------------------------------

void VirconRAM::LoadContents( const string& FilePath )
{
    // open the file
    LOG( "Loading RAM file \"" << FilePath << "\"" );
    
    ifstream InputFile;
    InputFile.open( FilePath, ios::binary | ios::ate );
    
    if( InputFile.fail() )
      THROW( "Cannot open RAM file" );
        
    // obtain file size
    int NumberOfBytes = InputFile.tellg();
    int NumberOfWords = NumberOfBytes / 4;
    LOG( "RAM size: " << NumberOfBytes << " bytes = " << NumberOfWords << " words" );
    
    // check size coherency
    if( NumberOfBytes != int(MemorySize * 4) )
    {
        InputFile.close();
        THROW( "Invalid RAM: File must match the size of the current RAM module" );
    }
    
    // load whole file to RAM
    InputFile.seekg( 0, ios::beg );
    InputFile.read( (char*)(&Memory[0]), MemorySize * 4 );
    
    // close the file
    InputFile.close();
}

// -----------------------------------------------------------------------------

void VirconRAM::ClearContents()
{
    memset( &Memory[ 0 ], 0, Memory.size() * 4 );
}

// -----------------------------------------------------------------------------

bool VirconRAM::ReadAddress( int32_t LocalAddress, VirconWord& Result )
{
    // check range
    if( LocalAddress >= MemorySize )
      return false;
    
    // provide value
    Result = Memory[ LocalAddress ];
    return true;
}

// -----------------------------------------------------------------------------

bool VirconRAM::WriteAddress( int32_t LocalAddress, VirconWord Value )
{
    // check range
    if( LocalAddress >= MemorySize )
      return false;
    
    // write value
    Memory[ LocalAddress ] = Value;
    return true;
}


// =============================================================================
//      CLASS: VIRCON ROM
// =============================================================================


VirconROM::VirconROM()
{
    MemorySize = 0;
}

// -----------------------------------------------------------------------------

void VirconROM::Connect( void* Source, uint32_t NumberOfWords )
{
    // first, remove any previous memory
    Disconnect();
    
    // resize ROM to new size
    Memory.resize( NumberOfWords );
    MemorySize = NumberOfWords;
    
    // copy the whole address space
    memcpy( &Memory[ 0 ], Source, NumberOfWords * 4 );
}

// -----------------------------------------------------------------------------

void VirconROM::Disconnect()
{
    Memory.clear();
    MemorySize = 0;
}

// -----------------------------------------------------------------------------

bool VirconROM::ReadAddress( int32_t LocalAddress, VirconWord& Result )
{
    // check range
    if( LocalAddress >= MemorySize )
      return false;
    
    // provide value
    Result = Memory[ LocalAddress ];
    return true;
}

// -----------------------------------------------------------------------------

bool VirconROM::WriteAddress( int32_t LocalAddress, VirconWord Value )
{
    // ROM cannot be written to
    return false;
}
