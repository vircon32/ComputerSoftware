// *****************************************************************************
    // include console logic headers
    #include "ExternalInterfaces.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CALLBACKS FOR EXTERNAL FUNCTIONS
    // =============================================================================
    
    
    namespace Callbacks
    {
        // callbacks to the video library
        void( *ClearScreen )( V32::GPUColor ) = nullptr;
        void( *DrawQuad )( V32::GPUQuad& ) = nullptr;
        void( *SetMultiplyColor )( V32::GPUColor ) = nullptr;
        void( *SetBlendingMode )( int ) = nullptr;
        void( *SelectTexture )( int ) = nullptr;
        void( *LoadTexture )( int, void* ) = nullptr;
        void( *UnloadCartridgeTextures )() = nullptr;
        
        // callbacks to the log library
        void( *LogLine )( const string& ) = nullptr;
        void( *ThrowException )( const string& ) = nullptr;
    }
}
