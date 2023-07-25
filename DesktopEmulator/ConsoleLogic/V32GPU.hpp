// *****************************************************************************
    // start include guard
    #ifndef V32GPU_HPP
    #define V32GPU_HPP
    
    // include project headers
    #include "V32Buses.hpp"
    #include "ExternalInterfaces.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      GPU DEFINITIONS
    // =============================================================================
    
    
    // LOCAL port numbers!
    // (to be added the initial address)
    enum class GPU_LocalPorts: int32_t
    {
        Command = 0,
        RemainingPixels,
        
        ClearColor,
        MultiplyColor,
        ActiveBlending,
        SelectedTexture,
        SelectedRegion,
        
        DrawingPointX,
        DrawingPointY,
        DrawingScaleX,
        DrawingScaleY,
        DrawingAngle,
        
        RegionMinX,
        RegionMinY,
        RegionMaxX,
        RegionMaxY,
        RegionHotspotX,
        RegionHotspotY
    };
    
    // used as limit of local port numbers
    const int32_t GPU_LastPort = (int32_t)GPU_LocalPorts::RegionHotspotY;
    
    
    // =============================================================================
    //      GPU INTERNAL DATA STRUCTURES
    // =============================================================================
    
    
    typedef struct
    {
        int32_t MinX, MinY;
        int32_t MaxX, MaxY;
        int32_t HotspotX, HotspotY;
    }
    GPURegion;
    
    // -----------------------------------------------------------------------------
    
    typedef struct
    {
        GPURegion Regions[ Constants::GPURegionsPerTexture ];
    }
    GPUTexture;
    
    
    // =============================================================================
    //      V32 GPU CLASS
    // =============================================================================
    
    
    class V32GPU: public VirconControlInterface
    {
        public:
            
            // textures loaded into GPU
            GPUTexture BiosTexture;
            std::vector< GPUTexture > CartridgeTextures;
            
            // accessors to active entities
            GPUTexture* PointedTexture;
            GPURegion*  PointedRegion;
            
            // GPU registers: GPU control
            int32_t Command;
            int32_t RemainingPixels;
            
            // GPU registers: global graphic parameters
            GPUColor   ClearColor;
            GPUColor   MultiplyColor;
            int32_t    ActiveBlending;
            int32_t    SelectedTexture;
            int32_t    SelectedRegion;
            
            // GPU registers: draw command parameters
            int32_t DrawingPointX;
            int32_t DrawingPointY;
            float   DrawingScaleX;
            float   DrawingScaleY;
            float   DrawingAngle;
            
            // quad coordinated for drawing regions
            GPUQuad RegionQuad;
            
            // external interfaces: video function callbacks
            void( *Callback_ClearScreen )( GPUColor );
            void( *Callback_DrawQuad )( GPUQuad& );
            void( *Callback_SetMultiplyColor )( GPUColor );
            void( *Callback_SetBlendingMode )( int );
            void( *Callback_SelectTexture )( int );
            void( *Callback_LoadTexture )( int, void* );
            void( *Callback_UnloadCartridgeTextures )();
            
        public:
            
            // instance handling
            V32GPU();
           ~V32GPU();
            
            // handling video resources
            void InsertCartridgeTextures( uint32_t NumberOfCartridgeTextures );
            void RemoveCartridgeTextures();
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
            
            // general operation
            void ChangeFrame();
            void Reset();
            
            // execution of GPU commands
            void ClearScreen();
            void DrawRegion( bool ScalingEnabled, bool RotationEnabled );
    };
    
    
    // =============================================================================
    //      GPU REGISTER WRITERS
    // =============================================================================
    
    
    bool WriteGPUCommand        ( V32GPU& GPU, V32Word Value );
    bool WriteGPURemainingPixels( V32GPU& GPU, V32Word Value );
    bool WriteGPUClearColor     ( V32GPU& GPU, V32Word Value );
    bool WriteGPUMultiplyColor  ( V32GPU& GPU, V32Word Value );
    bool WriteGPUActiveBlending ( V32GPU& GPU, V32Word Value );
    bool WriteGPUSelectedTexture( V32GPU& GPU, V32Word Value );
    bool WriteGPUSelectedRegion ( V32GPU& GPU, V32Word Value );
    bool WriteGPUDrawingPointX  ( V32GPU& GPU, V32Word Value );
    bool WriteGPUDrawingPointY  ( V32GPU& GPU, V32Word Value );
    bool WriteGPUDrawingScaleX  ( V32GPU& GPU, V32Word Value );
    bool WriteGPUDrawingScaleY  ( V32GPU& GPU, V32Word Value );
    bool WriteGPUDrawingAngle   ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionMinX     ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionMinY     ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionMaxX     ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionMaxY     ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionHotspotX ( V32GPU& GPU, V32Word Value );
    bool WriteGPURegionHotspotY ( V32GPU& GPU, V32Word Value );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
