// *****************************************************************************
    // start include guard
    #ifndef VIRCONBUSES_HPP
    #define VIRCONBUSES_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconDataStructures.hpp"
    
    // include C/C++ headers
    #include <vector>       // [ C++ STL ] Vectors
// *****************************************************************************


// forward declaration, since both types of buses
// will use pointers to CPU as their master device
class VirconCPU;


// =============================================================================
//      INTER-DEVICE BUS FOR ADDRESSING R/W ON MEMORY
// =============================================================================


class VirconMemoryInterface
{
    public:
        
        // R/W methods
        virtual bool ReadAddress( int32_t LocalAddress, VirconWord& Result ) = 0;
        virtual bool WriteAddress( int32_t LocalAddress, VirconWord Value  ) = 0;
};

// -----------------------------------------------------------------------------

class VirconMemoryBus
{
    public:
        
        // connected master
        VirconCPU* Master;
        
        // connected slaves
        VirconMemoryInterface* Slaves[ Constants::MemoryBusSlaves ];
        
    public:
        
        // instance handling
        VirconMemoryBus();
        
        // R/W methods
        bool ReadAddress( int32_t GlobalAddress, VirconWord& Result );
        bool WriteAddress( int32_t GlobalAddress, VirconWord Value );
};


// =============================================================================
//      INTER-DEVICE BUS FOR ADDRESSING R/W ON CONTROL PORTS
// =============================================================================


class VirconControlInterface
{
    public:
        
        // I/O port access
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result ) = 0;
        virtual bool WritePort( int32_t LocalPort, VirconWord Value ) = 0;
};

// -----------------------------------------------------------------------------

class VirconControlBus
{
    public:
        
        // connected master
        VirconCPU* Master;
        
        // connected slaves
        VirconControlInterface* Slaves[ Constants::ControlBusSlaves ];
        
    public:
        
        // instance handling
        VirconControlBus();
        
        // I/O port access
        void ReadPort( int32_t GlobalPort, VirconWord& Result );
        void WritePort( int32_t GlobalPort, VirconWord Value );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

