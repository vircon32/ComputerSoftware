// *****************************************************************************
    // start include guard
    #ifndef V32MEMORY_HPP
    #define V32MEMORY_HPP
    
    // include console logic headers
    #include "V32Buses.hpp"
    
    // include C/C++ headers
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      MODULE OF RANDOM ACCESS MEMORY
    // =============================================================================
    
    
    class V32RAM: public VirconMemoryInterface
    {
        public:
            
            std::vector< V32Word > Memory;
            int32_t MemorySize;
            
        public:
            
            // instance handling
            V32RAM();
            
            // memory connection
            void Connect( uint32_t NumberOfWords );
            void Disconnect();
            
            // memory contents
            void ClearContents();
            
            // bus connection
            virtual bool ReadAddress( int32_t LocalAddress, V32Word& Result );
            virtual bool WriteAddress( int32_t LocalAddress, V32Word Value );
    };
    
    
    // =============================================================================
    //      MODULE OF READ-ONLY MEMORY
    // =============================================================================
    
    
    class V32ROM: public VirconMemoryInterface
    {
        public:
            
            std::vector< V32Word > Memory;
            int32_t MemorySize;
            
        public:
            
            // instance handling
            V32ROM();
            
            // memory connection
            // (unlike RAM, we can only get the contents upon connection)
            void Connect( void* SourceData, uint32_t NumberOfWords );
            void Disconnect();
            
            // bus connection
            virtual bool ReadAddress( int32_t LocalAddress, V32Word& Result );
            virtual bool WriteAddress( int32_t LocalAddress, V32Word Value );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
