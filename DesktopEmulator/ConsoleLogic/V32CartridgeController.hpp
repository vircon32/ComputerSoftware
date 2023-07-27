// *****************************************************************************
    // start include guard
    #ifndef V32CARTRIDGECONTROLLER_HPP
    #define V32CARTRIDGECONTROLLER_HPP
    
    // include console logic headers
    #include "V32Buses.hpp"
    #include "V32Memory.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CARTRIDGE CONTROLLER DEFINITIONS
    // =============================================================================
    
    
    // LOCAL port numbers!
    // (to be added the initial address)
    enum class CAR_LocalPorts: int32_t
    {
        Connected = 0,
        ProgramROMSize,
        NumberOfTextures,
        NumberOfSounds
    };
    
    // used as limit of local port numbers
    const int32_t CAR_LastPort = (int32_t)CAR_LocalPorts::NumberOfSounds;
    
    
    // =============================================================================
    //      CARTRIDGE CONTROLLER CLASS
    // =============================================================================
    
    
    class V32CartridgeController: public VirconControlInterface, public V32ROM
    {
        public:
            
            // state of ports
            int32_t NumberOfTextures;
            int32_t NumberOfSounds;
            
            // additional data about the connected cartridge
            std::string CartridgeFileName;
            std::string CartridgeTitle;
            uint32_t CartridgeVersion;
            uint32_t CartridgeRevision;
            
        public:
            
            // instance handling
            V32CartridgeController();
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
