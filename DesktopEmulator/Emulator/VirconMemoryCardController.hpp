// *****************************************************************************
    // start include guard
    #ifndef VIRCONMEMORYCARDCONTROLLER_HPP
    #define VIRCONMEMORYCARDCONTROLLER_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
    #include "VirconMemory.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      STORAGE CONTROLLER DEFINITIONS
// =============================================================================


// LOCAL port numbers!
// (to be added the initial address)
enum class MEM_LocalPorts: int32_t
{
    Connected = 0
};

// used as limit of local port numbers
const int32_t MEM_LastPort = (int32_t)MEM_LocalPorts::Connected;


// =============================================================================
//      MEMORY CARD CONTROLLER CLASS
// =============================================================================


class VirconMemoryCardController: public VirconControlInterface, public VirconRAM
{
    public:
        
        // file save control
        std::string CardFileName;
        std::string CardSavePath;
        bool PendingSave;
        
    public:
        
        // instance handling
        VirconMemoryCardController();
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
        // connection to memory bus (overriden)
        virtual bool WriteAddress( int32_t LocalAddress, VirconWord Value );
        
        // memory contents (overriden)
        virtual void LoadContents( const std::string& FilePath );
        virtual void SaveContents( const std::string& FilePath );
        
        // additional method for creating a new file
        void CreateNewFile( const std::string& FilePath );
        
        // general operation
        void ChangeFrame();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
