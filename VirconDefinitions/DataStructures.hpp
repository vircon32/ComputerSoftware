// *****************************************************************************
    // start include guard
    #ifndef V32DATASTRUCTURES_HPP
    #define V32DATASTRUCTURES_HPP
    
    // include C/C++ headers
    #include <cstdint>       // [ ANSI C ] Standard integer types
// -----------------------------------------------------------------------------
//  This file declares the core Vircon32 data structures, that are used
//  together for the central definition: a system-wide 32-bit word.
//  This word is the common format to transfer data between components
//  (through buses). CPU, GPU, etc will each use different aspects of it.
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


namespace V32
{
    // =============================================================================
    //      CONSOLE-LEVEL DEFINITIONS
    // =============================================================================
    
    
    // ordering: opcode is in the 6 most significant bits
    typedef struct
    {
        unsigned int PortNumber : 14;
        unsigned int AddressingMode : 3;
        unsigned int Register2 : 4;
        unsigned int Register1 : 4;
        unsigned int UsesImmediate : 1;
        unsigned int OpCode : 6;
    }
    CPUInstruction;
    
    // -----------------------------------------------------------------------------
    
    // ordering: "RGBA", from most to least significant bytes     // CHECK WITH PNG->RGBA EXAMPLES
    //   byte 0   byte 1   byte 2   byte 3
    //    Red      Green    Blue     Alpha
    // this is the order used in both PNG files and OpenGL RGBA
    typedef struct
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t A;
    }
    GPUColor;
    
    // -----------------------------------------------------------------------------
    
    // ordering, from most to least significant:          // CHECK WITH WAV EXAMPLES
    //   byte 0    byte 1     byte 2    byte 3  
    // [Left LSB][Left MSB][Right LSB][Right MSB]
    // this is the order used in 16-bit stereo WAV files
    typedef struct
    {
        int16_t LeftSample;
        int16_t RightSample;
    }
    SPUSample;
    
    
    // =============================================================================
    //      DEFINITION OF THE COMMON, SYSTEM-LEVEL 32-BIT WORD
    // =============================================================================
    
    
    // note that all Vircon 32-bit primitive types are little endian
    typedef union
    {
        float           AsFloat;
        int32_t         AsInteger;
        uint32_t        AsBinary;
        CPUInstruction  AsInstruction;
        GPUColor        AsColor;
        SPUSample       AsSample;
    }
    VirconWord;
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
