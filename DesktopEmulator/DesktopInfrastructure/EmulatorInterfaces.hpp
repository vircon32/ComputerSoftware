// *****************************************************************************
    // start include guard
    #ifndef EMULATORINTERFACES_HPP
    #define EMULATORINTERFACES_HPP
    
    // include common vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
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
        float x, y;
    }
    GPUPoint;
    
    // -----------------------------------------------------------------------------
    
    typedef struct
    {
        GPUPoint VertexPositions[ 4 ];
        GPUPoint VertexTexCoords[ 4 ];
    }
    GPUQuad;
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
