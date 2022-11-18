// *****************************************************************************
    // start include guard
    #ifndef VIRCONNULLCONTROLLER_HPP
    #define VIRCONNULLCONTROLLER_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
// *****************************************************************************


// =============================================================================
//      DUMMY CONTROLLER FOR NON-EXISTENT PORTS
// =============================================================================


class VirconNullController: public VirconControlInterface
{
    public:
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

