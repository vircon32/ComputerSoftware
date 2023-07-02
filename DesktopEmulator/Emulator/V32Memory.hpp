// *****************************************************************************
    // start include guard
    #ifndef V32MEMORY_HPP
    #define V32MEMORY_HPP
    
    // include project headers
    #include "V32Buses.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      MODULE OF RANDOM ACCESS MEMORY
// =============================================================================


class V32RAM: public VirconMemoryInterface
{
    public:
        
        std::vector< VirconWord > Memory;
        int32_t MemorySize;
        
    public:
        
        // instance handling
        V32RAM();
        
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


class V32ROM: public VirconMemoryInterface
{
    public:
        
        std::vector< VirconWord > Memory;
        int32_t MemorySize;
        
    public:
        
        // instance handling
        V32ROM();
        
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
