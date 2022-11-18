// *****************************************************************************
    // start include guard
    #ifndef VIRCONRNG_HPP
    #define VIRCONRNG_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
// *****************************************************************************


// =============================================================================
//      RNG DEFINITIONS
// =============================================================================


// local port numbers
enum class RNG_LocalPorts: int32_t
{
    CurrentValue = 0
};


// =============================================================================
//      VIRCON RANDOM NUMBER GENERATOR
// =============================================================================


class VirconRNG: public VirconControlInterface
{
    public:
        
        int32_t CurrentValue;
        
    public:
        
        // instance handling
        VirconRNG();
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
        // general operation
        void Reset();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
