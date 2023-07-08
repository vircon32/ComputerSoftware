// *****************************************************************************
    // start include guard
    #ifndef V32GPU_HPP
    #define V32GPU_HPP
    
    // include project headers
    #include "V32Buses.hpp"
    #include "ExternalInterfaces.hpp"
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
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
        GLuint TextureID;
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
            
        public:
            
            // instance handling
            V32GPU();
           ~V32GPU();
            
            // handling video resources
            void LoadTexture( GPUTexture& TargetTexture, void* Pixels, unsigned Width, unsigned Height );
            void UnloadTexture( GPUTexture& TargetTexture );
            
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
    
    
    void WriteGPUCommand        ( V32GPU& GPU, V32Word Value );
    void WriteGPURemainingPixels( V32GPU& GPU, V32Word Value );
    void WriteGPUClearColor     ( V32GPU& GPU, V32Word Value );
    void WriteGPUMultiplyColor  ( V32GPU& GPU, V32Word Value );
    void WriteGPUActiveBlending ( V32GPU& GPU, V32Word Value );
    void WriteGPUSelectedTexture( V32GPU& GPU, V32Word Value );
    void WriteGPUSelectedRegion ( V32GPU& GPU, V32Word Value );
    void WriteGPUDrawingPointX  ( V32GPU& GPU, V32Word Value );
    void WriteGPUDrawingPointY  ( V32GPU& GPU, V32Word Value );
    void WriteGPUDrawingScaleX  ( V32GPU& GPU, V32Word Value );
    void WriteGPUDrawingScaleY  ( V32GPU& GPU, V32Word Value );
    void WriteGPUDrawingAngle   ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionMinX     ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionMinY     ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionMaxX     ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionMaxY     ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionHotspotX ( V32GPU& GPU, V32Word Value );
    void WriteGPURegionHotspotY ( V32GPU& GPU, V32Word Value );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
