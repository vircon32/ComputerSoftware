// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/Logger.hpp"
    
    // include project headers
    #include "V32Memory.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CLASS: V32 RAM
    // =============================================================================
    
    
    V32RAM::V32RAM()
    {
        MemorySize = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32RAM::Connect( uint32_t NumberOfWords )
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
    
    void V32RAM::Disconnect()
    {
        Memory.clear();
        MemorySize = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32RAM::ClearContents()
    {
        memset( &Memory[ 0 ], 0, Memory.size() * 4 );
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32RAM::ReadAddress( int32_t LocalAddress, V32Word& Result )
    {
        // check range
        if( LocalAddress >= MemorySize )
          return false;
        
        // provide value
        Result = Memory[ LocalAddress ];
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32RAM::WriteAddress( int32_t LocalAddress, V32Word Value )
    {
        // check range
        if( LocalAddress >= MemorySize )
          return false;
        
        // write value
        Memory[ LocalAddress ] = Value;
        return true;
    }
    
    
    // =============================================================================
    //      CLASS: V32 ROM
    // =============================================================================
    
    
    V32ROM::V32ROM()
    {
        MemorySize = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32ROM::Connect( void* Source, uint32_t NumberOfWords )
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
    
    void V32ROM::Disconnect()
    {
        Memory.clear();
        MemorySize = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32ROM::ReadAddress( int32_t LocalAddress, V32Word& Result )
    {
        // check range
        if( LocalAddress >= MemorySize )
          return false;
        
        // provide value
        Result = Memory[ LocalAddress ];
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32ROM::WriteAddress( int32_t LocalAddress, V32Word Value )
    {
        // ROM cannot be written to
        return false;
    }
}
