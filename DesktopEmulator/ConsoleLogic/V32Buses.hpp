// *****************************************************************************
    // start include guard
    #ifndef V32BUSES_HPP
    #define V32BUSES_HPP
    
    // include common Vircon32 headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/DataStructures.hpp"
// *****************************************************************************


namespace V32
{
    // forward declaration, since both types of buses
    // will use pointers to CPU as their master device
    class V32CPU;
    
    
    // =============================================================================
    //      INTER-DEVICE BUS FOR ADDRESSING R/W ON MEMORY
    // =============================================================================
    
    
    class VirconMemoryInterface
    {
        public:
            
            // R/W methods
            virtual bool ReadAddress( int32_t LocalAddress, V32Word& Result ) = 0;
            virtual bool WriteAddress( int32_t LocalAddress, V32Word Value  ) = 0;
    };
    
    // -----------------------------------------------------------------------------
    
    class V32MemoryBus
    {
        public:
            
            // connected master
            V32CPU* Master;
            
            // connected slaves
            VirconMemoryInterface* Slaves[ Constants::MemoryBusSlaves ];
            
        public:
            
            // instance handling
            V32MemoryBus();
            
            // R/W methods
            void ReadAddress( int32_t GlobalAddress, V32Word& Result );
            void WriteAddress( int32_t GlobalAddress, V32Word Value );
    };
    
    
    // =============================================================================
    //      INTER-DEVICE BUS FOR ADDRESSING R/W ON CONTROL PORTS
    // =============================================================================
    
    
    class VirconControlInterface
    {
        public:
            
            // I/O port access
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result ) = 0;
            virtual bool WritePort( int32_t LocalPort, V32Word Value ) = 0;
    };
    
    // -----------------------------------------------------------------------------
    
    class V32ControlBus
    {
        public:
            
            // connected master
            V32CPU* Master;
            
            // connected slaves
            VirconControlInterface* Slaves[ Constants::ControlBusSlaves ];
            
        public:
            
            // instance handling
            V32ControlBus();
            
            // I/O port access
            void ReadPort( int32_t GlobalPort, V32Word& Result );
            void WritePort( int32_t GlobalPort, V32Word Value );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
