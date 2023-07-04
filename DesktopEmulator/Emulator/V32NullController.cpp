// *****************************************************************************
    // include project headers
    #include "V32NullController.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CLASS: V32 NULL CONTROLLER
    // =============================================================================
    
    
    bool V32NullController::ReadPort( int32_t LocalPort, VirconWord& Result )
    {
        // ports can never be read from
        return false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32NullController::WritePort( int32_t LocalPort, VirconWord Value )
    {
        // ports can never be written to
        return false;
    }
}
