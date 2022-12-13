// *****************************************************************************
    // start include guard
    #ifndef VIRCONGPU_HPP
    #define VIRCONGPU_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


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
//      GPU DATA STRUCTURES
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
//      VIRCON GPU CLASS
// =============================================================================


class VirconGPU: public VirconControlInterface
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
        
    public:
        
        // instance handling
        VirconGPU();
       ~VirconGPU();
        
        // handling video resources
        void LoadTexture( GPUTexture& TargetTexture, void* Pixels, unsigned Width, unsigned Height );
        void UnloadTexture( GPUTexture& TargetTexture );
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
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


void WriteGPUCommand        ( VirconGPU& GPU, VirconWord Value );
void WriteGPURemainingPixels( VirconGPU& GPU, VirconWord Value );
void WriteGPUClearColor     ( VirconGPU& GPU, VirconWord Value );
void WriteGPUMultiplyColor  ( VirconGPU& GPU, VirconWord Value );
void WriteGPUActiveBlending ( VirconGPU& GPU, VirconWord Value );
void WriteGPUSelectedTexture( VirconGPU& GPU, VirconWord Value );
void WriteGPUSelectedRegion ( VirconGPU& GPU, VirconWord Value );
void WriteGPUDrawingPointX  ( VirconGPU& GPU, VirconWord Value );
void WriteGPUDrawingPointY  ( VirconGPU& GPU, VirconWord Value );
void WriteGPUDrawingScaleX  ( VirconGPU& GPU, VirconWord Value );
void WriteGPUDrawingScaleY  ( VirconGPU& GPU, VirconWord Value );
void WriteGPUDrawingAngle   ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionMinX     ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionMinY     ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionMaxX     ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionMaxY     ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionHotspotX ( VirconGPU& GPU, VirconWord Value );
void WriteGPURegionHotspotY ( VirconGPU& GPU, VirconWord Value );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
