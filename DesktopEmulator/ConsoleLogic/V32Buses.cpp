// *****************************************************************************
    // include project headers
    #include "V32Buses.hpp"
    #include "V32CPU.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CLASS: V32 MEMORY BUS
    // =============================================================================
    
    
    V32MemoryBus::V32MemoryBus()
    {
        Master = nullptr;
        
        for( int i = 0; i < Constants::MemoryBusSlaves; i++ )
          Slaves[ i ] = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32MemoryBus::ReadAddress( int32_t GlobalAddress, V32Word& Result )
    {
        // separate device ID and local address
        int32_t DeviceID = (GlobalAddress >> 28) & 3;
        int32_t LocalAddress = GlobalAddress & 0x0FFFFFFF;
        
        // attempt to read from memory
        bool Success = Slaves[ DeviceID ]->ReadAddress( LocalAddress, Result );
        
        // raise a CPU error when it failed
        if( !Success )
          Master->RaiseHardwareError( CPUErrorCodes::InvalidMemoryRead );
        
        return Success;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32MemoryBus::WriteAddress( int32_t GlobalAddress, V32Word Value )
    {
        // separate device ID and local address
        int32_t DeviceID = (GlobalAddress >> 28) & 3;
        int32_t LocalAddress = GlobalAddress & 0x0FFFFFFF;
        
        // attempt to write on memory
        bool Success = Slaves[ DeviceID ]->WriteAddress( LocalAddress, Value );
        
        // raise a CPU error when it failed
        if( !Success )
          Master->RaiseHardwareError( CPUErrorCodes::InvalidMemoryWrite );
        
        return Success;
    }
    
    
    // =============================================================================
    //      CLASS: V32 CONTROL BUS
    // =============================================================================
    
    
    V32ControlBus::V32ControlBus()
    {
        Master = nullptr;
        
        for( int i = 0; i < Constants::ControlBusSlaves; i++ )
          Slaves[ i ] = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32ControlBus::ReadPort( int32_t GlobalPort, V32Word& Result )
    {
        // separate device ID and local address
        int32_t DeviceID = (GlobalPort >> 8) & 7;
        int32_t LocalPort = GlobalPort & 0xFF;
        
        // attempt to read from port
        bool Success = Slaves[ DeviceID ]->ReadPort( LocalPort, Result );
        
        // raise a CPU error when it failed
        if( !Success )
          Master->RaiseHardwareError( CPUErrorCodes::InvalidPortRead );
    }
    
    // -----------------------------------------------------------------------------
    
    void V32ControlBus::WritePort( int32_t GlobalPort, V32Word Value )
    {
        // separate device ID and local address
        int32_t DeviceID = (GlobalPort >> 8) & 7;
        int32_t LocalPort = GlobalPort & 0xFF;
        
        // attempt to write on port
        bool Success = Slaves[ DeviceID ]->WritePort( LocalPort, Value );
        
        // raise a CPU error when it failed
        if( !Success )
          Master->RaiseHardwareError( CPUErrorCodes::InvalidPortWrite );
    }
}
