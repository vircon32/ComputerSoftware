// *****************************************************************************
    // include project headers
    #include "ExternalInterfaces.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      SETTING VIDEO CALLBACK FUNCTIONS
// =============================================================================


namespace V32
{
    namespace Callbacks
    {
        void( *ClearScreen )( V32::GPUColor ) = nullptr;
        void( *DrawQuad )( V32::GPUQuad& ) = nullptr;
        void( *SetMultiplyColor )( V32::GPUColor ) = nullptr;
        void( *SetBlendingMode )( int ) = nullptr;
        void( *SelectTexture )( int ) = nullptr;
        void( *LoadTexture )( int, void* ) = nullptr;
        void( *UnloadCartridgeTextures )() = nullptr;
    }
}
