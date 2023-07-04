// *****************************************************************************
    // include project headers
    #include "V32CartridgeController.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CLASS: V32 CARTRIDGE CONTROLLER
    // =============================================================================
    
    
    V32CartridgeController::V32CartridgeController()
    {
        // initial state of ports
        NumberOfTextures = 0;
        NumberOfSounds = 0;
        
        // no cartridge information
        CartridgeTitle = "";
        CartridgeVersion = 0;
        CartridgeRevision = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32CartridgeController::ReadPort( int32_t LocalPort, V32Word& Result )
    {
        // check range
        if( LocalPort > CAR_LastPort )
          return false;
        
        // provide value
        if( LocalPort == (int32_t)CAR_LocalPorts::Connected )
          Result.AsInteger = (MemorySize > 0? 1 : 0);
        
        else if( LocalPort == (int32_t)CAR_LocalPorts::ProgramROMSize )
          Result.AsInteger = MemorySize;
        
        else if( LocalPort == (int32_t)CAR_LocalPorts::NumberOfTextures )
          Result.AsInteger = NumberOfTextures;
        
        else
          Result.AsInteger = NumberOfSounds;
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32CartridgeController::WritePort( int32_t LocalPort, V32Word Value )
    {
        // all these registers are read-only, so just ignore the request
        return false;
    }
}
