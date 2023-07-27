// *****************************************************************************
    // include project headers
    #include "V32MemoryCardController.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      V32 MEMORY CARD CONTROLLER: OWN METHODS
    // =============================================================================
    
    
    V32MemoryCardController::V32MemoryCardController()
    {
        PendingSave = false;
    }
    
    // -----------------------------------------------------------------------------
    
    V32MemoryCardController::~V32MemoryCardController()
    {
        // ensure we always close the file
        if( LinkedFile.is_open() )
          LinkedFile.close();
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32MemoryCardController::ReadPort( int32_t LocalPort, V32Word& Result )
    {
        // check range
        if( LocalPort > MEM_LastPort )
          return false;
        
        // provide value
        if( LocalPort == (int32_t)MEM_LocalPorts::Connected )
          Result.AsInteger = (MemorySize > 0? 1 : 0);
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32MemoryCardController::WritePort( int32_t LocalPort, V32Word Value )
    {
        // all memory card ports are read-only!
        return false;
    }
    
    
    // =============================================================================
    //      V32 MEMORY CARD CONTROLLER: METHODS OVERRIDEN FROM RAM
    // =============================================================================
    
    
    bool V32MemoryCardController::WriteAddress( int32_t LocalAddress, V32Word Value )
    {
        // check that the normal RAM write is successful
        if( !V32RAM::WriteAddress( LocalAddress, Value ) )
          return false;
        
        // data is now pending to save
        PendingSave = true;
        
        return true;
    }
}
