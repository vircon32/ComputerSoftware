// *****************************************************************************
    // start include guard
    #ifndef VIRCONCARTRIDGECONTROLLER_HPP
    #define VIRCONCARTRIDGECONTROLLER_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
    #include "VirconMemory.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


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


class VirconCartridgeController: public VirconControlInterface, public VirconROM
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
        VirconCartridgeController();
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
