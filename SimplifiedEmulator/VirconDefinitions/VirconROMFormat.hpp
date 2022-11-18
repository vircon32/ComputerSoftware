// *****************************************************************************
    // start include guard
    #ifndef VIRCONROMFORMAT_HPP
    #define VIRCONROMFORMAT_HPP
    
    // include C/C++ headers
    #include <cstdint>          // [ ANSI C ] Standard integer types
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
// -----------------------------------------------------------------------------
//  This file defines the way in which ROM files are packed.
//  These headers are used by packing and unpacking programs.
//  Note that 16/32-bit fields are affected by endianness.
// *****************************************************************************


// enforce the definition of host system's endianness
// (otherwise, correctness of binary ABI cannot be guaranteed)
#ifndef __BYTE_ORDER__
#error "Endianness is not defined"
#endif

// Vircon32 is little endian, both in its internal
// data and in its external ROM/save file formats.
// For now we will demand host to be little endian.
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Current Vircon tools only exist for little endian systems"
#endif


// =============================================================================
//      FILE SIGNATURES
// =============================================================================


// all of these signatures are exactly 8 bytes
// (i.e. none of them must be null terminated)
namespace Signatures
{
    // signatures expected to indicate ROM type
    // (every file will must either one or the other)
    const char CartridgeFile[] = "V32-CART";
    const char BiosFile[]      = "V32-BIOS";
    
    // signatures for the separate binary files
    const char BinaryFile[]  = "V32-VBIN";
    const char TextureFile[] = "V32-VTEX";
    const char SoundFile[]   = "V32-VSND";
    
    // signature for memory card save files
    const char MemoryCardFile[] = "V32-MEMC";
}

// -----------------------------------------------------------------------------

// functions to work with signatures
void WriteSignature( std::ofstream& OutputFile, const char* Value );
bool CheckSignature( char* Signature, const char* Expected );


// =============================================================================
//      GLOBAL HEADER IN A ROM FILE
// =============================================================================


// Pointers to each section are given in this format.
// When an optional section does not exist, length is 0.
typedef struct
{
    uint32_t StartOffset;  // given in bytes from the start of file
    uint32_t Length;       // given in bytes
}
SectionLocation;

// -----------------------------------------------------------------------------

// initial header; must be placed at the beginning of
// the file, and be in size exactly 128 bytes = 0x80)
typedef struct
{
    // Vircon32 metadata
    char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
    uint32_t VirconVersion;
    uint32_t VirconRevision;    
    
    // ROM metadata
    char Title[ 64 ];           // must have null termination (i.e. up to 63 characters)
    uint32_t ROMVersion;
    uint32_t ROMRevision;
    
    // data on ROM contents
    uint32_t NumberOfTextures;
    uint32_t NumberOfSounds;
    SectionLocation ProgramROMLocation;
    SectionLocation VideoROMLocation;
    SectionLocation AudioROMLocation;
    
    // unused extra space
    int8_t Reserved[ 8 ];       // reserved for possible use in future versions
}
ROMFileHeader;


// =============================================================================
//      FILE HEADERS FOR SEPARATE BINARY FILES
// =============================================================================


typedef struct
{
    char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
    uint32_t NumberOfWords;     // length of the binary data in words
}
BinaryFileHeader;

// -----------------------------------------------------------------------------

typedef struct
{
    char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
    uint32_t TextureWidth;      // texture width in pixels
    uint32_t TextureHeight;     // texture height in pixels
}
TextureFileHeader;

// -----------------------------------------------------------------------------

typedef struct
{
    char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
    uint32_t SoundSamples;      // length of the sound in samples
}
SoundFileHeader;


// =============================================================================
//      FILE HEADER FOR MEMORY CARD FILES
// =============================================================================


// other than the file signature, no additional information is needed
typedef struct
{
    char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
}
MemoryCardFileHeader;


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

