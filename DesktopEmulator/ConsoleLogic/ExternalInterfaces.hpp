// *****************************************************************************
    // start include guard
    #ifndef EXTERNALINTERFACES_HPP
    #define EXTERNALINTERFACES_HPP
    
    // include common vircon32 headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/DataStructures.hpp"
    
    // include C/C++ headers
    #include <string>         // [ C++ STL ] Strings
    #include <stdexcept>      // [ C++ STL ] Exceptions
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      DEFINITIONS FOR GAMEPAD CONTROLLER INTERFACES
    // =============================================================================
    
    
    // gamepad controls for input events
    enum class GamepadControls
    {
        Left = 0,
        Right,
        Up,
        Down,
        ButtonStart,
        ButtonA,
        ButtonB,
        ButtonX,
        ButtonY,
        ButtonL,
        ButtonR
    };
    
    
    // =============================================================================
    //      DEFINITIONS FOR SPU INTERFACES
    // =============================================================================
    
    
    // define the sound buffers used for SPU audio output
    typedef struct
    {
        int32_t SequenceNumber;
        SPUSample Samples[ Constants::SPUSamplesPerFrame ];
    }
    SPUOutputBuffer;
    
    
    // =============================================================================
    //      DEFINITIONS FOR GPU INTERFACES
    // =============================================================================
    
    
    typedef struct
    {
        float x, y, texture_x, texture_y;
    }
    GPUPoint;
    
    // -----------------------------------------------------------------------------
    
    typedef struct
    {
        GPUPoint Vertices[ 4 ];
    }
    GPUQuad;
    
    
    // =============================================================================
    //      CALLBACKS FOR EXTERNAL FUNCTIONS
    // =============================================================================
    
    
    // note that providing all these callbacks is required:
    // the console will invoke them without any checks
    namespace Callbacks
    {
        // callbacks to the video library
        extern void( *ClearScreen )( V32::GPUColor );
        extern void( *DrawQuad )( V32::GPUQuad& );
        extern void( *SetMultiplyColor )( V32::GPUColor );
        extern void( *SetBlendingMode )( int );
        extern void( *SelectTexture )( int );
        extern void( *LoadTexture )( int, void* );
        extern void( *UnloadCartridgeTextures )();
        extern void( *UnloadBiosTexture )();
        
        // callbacks to the log library
        extern void( *LogLine )( const std::string& );
        extern void( *ThrowException )( const std::string& );
    }
    
    
    // =============================================================================
    //      SPECIFIC CPU EXCEPTIONS
    // =============================================================================
    
    
    // these will be thrown when a CPU hardware error
    // is raised, to stop any other pending execution
    class CPUException: public std::exception
    {
        public:
            
            CPUException() {};
            virtual ~CPUException() noexcept {};
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
