// *****************************************************************************
    // start include guard
    #ifndef V32NULLCONTROLLER_HPP
    #define V32NULLCONTROLLER_HPP
    
    // include project headers
    #include "V32Buses.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      DUMMY CONTROLLER FOR NON-EXISTENT PORTS
    // =============================================================================
    
    
    class V32NullController: public VirconControlInterface
    {
        public:
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
