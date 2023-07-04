// *****************************************************************************
    // start include guard
    #ifndef V32FILEFORMATS_HPP
    #define V32FILEFORMATS_HPP
    
    // include C/C++ headers
    #include <cstdint>      // [ ANSI C ] Standard integer types
// -----------------------------------------------------------------------------
//  This file defines the way in which ROM and RAM files are packed.
//  These headers are used by packing and unpacking programs. Note
//  that 16/32-bit fields are affected by endianness.
// *****************************************************************************


// We need to enforce host systems to have a defined endianness
// (otherwise, correctness of binary ABI cannot be guaranteed).
// Vircon32 is little endian, both in its internal data and in
// its external ROM/card file formats. For now we will demand
// hosts to be little endian.
#if defined(__BYTE_ORDER__)

  #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
  #error "Current Vircon32 tools only exist for little endian systems"
  #endif

#elif defined(__BIG_ENDIAN__)
  #error "Current Vircon32 tools only exist for little endian systems"

// we will assume all current Windows systems to be little endian
#elif !defined(WIN32) && !defined(WIN64) && !defined(__LITTLE_ENDIAN__)
  #error "Endianness is not defined"

#endif


namespace V32
{
    // =============================================================================
    //      FORMAT FOR SEPARATE BINARY FILES
    // =============================================================================
    
    
    namespace BinaryFileFormat
    {
        // expected file signature
        const char Signature[]  = "V32-VBIN";
        
        // initial header; must be placed at the beginning of
        // the file, and be a size of exactly 12 bytes
        typedef struct
        {
            char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
            uint32_t NumberOfWords;     // length of the binary data in words
        }
        Header;
    }
    
    // -----------------------------------------------------------------------------
    
    namespace TextureFileFormat
    {
        // expected file signature
        const char Signature[]  = "V32-VTEX";
        
        // initial header; must be placed at the beginning of
        // the file, and be a size of exactly 16 bytes
        typedef struct
        {
            char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
            uint32_t TextureWidth;      // texture width in pixels
            uint32_t TextureHeight;     // texture height in pixels
        }
        Header;
    }
    
    // -----------------------------------------------------------------------------
    
    namespace SoundFileFormat
    {
        // expected file signature
        const char Signature[]  = "V32-VSND";
        
        // initial header; must be placed at the beginning of
        // the file, and be a size of exactly 14 bytes
        typedef struct
        {
            char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
            uint32_t SoundSamples;      // length of the sound in samples
        }
        Header;
    }
    
    
    // =============================================================================
    //      FORMAT FOR CARTRIDGE & BIOS ROM FILES
    // =============================================================================
    
    
    namespace ROMFileFormat
    {
        // signatures expected to indicate ROM type
        // (every file will must either one or the other)
        const char CartridgeSignature[] = "V32-CART";
        const char BiosSignature[]      = "V32-BIOS";
        
        // Pointers to each section are given in this format.
        // When an optional section does not exist, length is 0.
        typedef struct
        {
            uint32_t StartOffset;  // given in bytes from the start of file
            uint32_t Length;       // given in bytes
        }
        SectionLocation;
        
        // initial header; must be placed at the beginning of
        // the file, and be a size of exactly 128 bytes = 0x80
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
        Header;
    }
    
    
    // =============================================================================
    //      FORMAT FOR MEMORY CARD RAM FILES
    // =============================================================================
    
    
    namespace MemoryCardFileFormat
    {
        // expected file signature
        const char Signature[]  = "V32-MEMC";
        
        // initial header; must be placed at the beginning of
        // the file, and be a size of exactly 8 bytes (other
        // than the file signature, no information is needed)
        typedef struct
        {
            char Signature[ 8 ];        // no null termination! (always taken as 8 characters)
        }
        Header;
    }
    
    
    // =============================================================================
    //      COMPILE-TIME CHECKS FOR HEADER SIZES
    // =============================================================================
    
    
    static_assert( sizeof(BinaryFileFormat::Header) == 12, "Wrong size for binary file header" );
    static_assert( sizeof(TextureFileFormat::Header)  == 16, "Wrong size for texture file header" );
    static_assert( sizeof(SoundFileFormat::Header) == 12, "Wrong size for sound file header" );
    static_assert( sizeof(ROMFileFormat::Header) == 128, "Wrong size for ROM file header" );
    static_assert( sizeof(MemoryCardFileFormat::Header) == 8, "Wrong size for memory card file header" );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
