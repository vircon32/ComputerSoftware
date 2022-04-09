// *****************************************************************************
    // start include guard
    #ifndef MEMORYPLACEMENT_HPP
    #define MEMORYPLACEMENT_HPP
    
    // include C/C++ headers
    #include <stdint.h>     // [ ANSI C ] Fixed-size integers
    #include <string>       // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      REPRESENTATION OF VARIABLE ADDRESSES
// =============================================================================


class MemoryPlacement
{
    public:
        
        // for global placement
        bool IsGlobal;
        int32_t GlobalAddress;   // signed, but cannot be negative
        std::string GlobalName;
        bool UsesGlobalName;
        
        // for local placement
        int32_t OffsetFromBP;    // may be negative
        
        // for embedded placement
        bool IsEmbedded;
        std::string EmbeddedName;
        
    public:
        
        // instance handling
        MemoryPlacement();
        
        // general operation
        void AddOffset( int Offset );
        std::string AccessAddressString();
        std::string PassingAddressString();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
