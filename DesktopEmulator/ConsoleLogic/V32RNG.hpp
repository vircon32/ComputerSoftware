// *****************************************************************************
    // start include guard
    #ifndef V32RNG_HPP
    #define V32RNG_HPP
    
    // include console logic headers
    #include "V32Buses.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      RNG DEFINITIONS
    // =============================================================================
    
    
    // local port numbers
    enum class RNG_LocalPorts: int32_t
    {
        CurrentValue = 0
    };
    
    
    // =============================================================================
    //      V32 RANDOM NUMBER GENERATOR
    // =============================================================================
    
    
    class V32RNG: public VirconControlInterface
    {
        public:
            
            int32_t CurrentValue;
            
        public:
            
            // instance handling
            V32RNG();
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
            
            // general operation
            void Reset();
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
