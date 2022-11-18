// *****************************************************************************
    // include project headers
    #include "VirconBuses.hpp"
    #include "VirconCPU.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: VIRCON MEMORY BUS
// =============================================================================


VirconMemoryBus::VirconMemoryBus()
{
    Master = nullptr;
    
    for( int i = 0; i < Constants::MemoryBusSlaves; i++ )
      Slaves[ i ] = nullptr;
}

// -----------------------------------------------------------------------------

bool VirconMemoryBus::ReadAddress( int32_t GlobalAddress, VirconWord& Result )
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

bool VirconMemoryBus::WriteAddress( int32_t GlobalAddress, VirconWord Value )
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
//      CLASS: VIRCON CONTROL BUS
// =============================================================================


VirconControlBus::VirconControlBus()
{
    Master = nullptr;
    
    for( int i = 0; i < Constants::ControlBusSlaves; i++ )
      Slaves[ i ] = nullptr;
}

// -----------------------------------------------------------------------------

void VirconControlBus::ReadPort( int32_t GlobalPort, VirconWord& Result )
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

void VirconControlBus::WritePort( int32_t GlobalPort, VirconWord Value )
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
