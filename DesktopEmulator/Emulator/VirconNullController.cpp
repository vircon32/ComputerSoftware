// *****************************************************************************
    // include project headers
    #include "VirconNullController.hpp"
// *****************************************************************************


// =============================================================================
//      CLASS: VIRCON NULL CONTROLLER
// =============================================================================


bool VirconNullController::ReadPort( int32_t LocalPort, VirconWord& Result )
{
    // ports can never be read from
    return false;
}

// -----------------------------------------------------------------------------

bool VirconNullController::WritePort( int32_t LocalPort, VirconWord Value )
{
    // ports can never be written to
    return false;
}
