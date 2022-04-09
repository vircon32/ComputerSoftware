// *****************************************************************************
    // start include guard
    #ifndef VIRCONMEMORY_HPP
    #define VIRCONMEMORY_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      MODULE OF RANDOM ACCESS MEMORY
// =============================================================================


class VirconRAM: public VirconMemoryInterface
{
    public:
        
        std::vector< VirconWord > Memory;
        int32_t MemorySize;
        
    public:
        
        // instance handling
        VirconRAM();
        
        // memory connection
        void Connect( uint32_t NumberOfWords );
        void Disconnect();
        
        // memory contents
        virtual void SaveContents( const std::string& FilePath );
        virtual void LoadContents( const std::string& FilePath );
        void ClearContents();
        
        // bus connection
        virtual bool ReadAddress( int32_t LocalAddress, VirconWord& Result );
        virtual bool WriteAddress( int32_t LocalAddress, VirconWord Value );
};


// =============================================================================
//      MODULE OF READ-ONLY MEMORY
// =============================================================================


class VirconROM: public VirconMemoryInterface
{
    public:
        
        std::vector< VirconWord > Memory;
        int32_t MemorySize;
        
    public:
        
        // instance handling
        VirconROM();
        
        // memory connection
        // (unlike RAM, we can only get the contents upon connection)
        void Connect( void* SourceData, uint32_t NumberOfWords );
        void Disconnect();
        
        // bus connection
        virtual bool ReadAddress( int32_t LocalAddress, VirconWord& Result );
        virtual bool WriteAddress( int32_t LocalAddress, VirconWord Value );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
