// *****************************************************************************
    // include common Vircon32 headers
    #include "../VirconDefinitions/Constants.hpp"
    #include "../VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "V32GPU.hpp"
    #include "ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Mathematics
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      PORT WRITERS TABLE
    // =============================================================================
    
    
    typedef bool (*GPUPortWriter)( V32GPU&, V32Word );
    
    // -----------------------------------------------------------------------------
    
    // dispatch vector table for all 18 GPU ports
    const GPUPortWriter GPUPortWriterTable[] =
    {
        WriteGPUCommand,
        WriteGPURemainingPixels,
        WriteGPUClearColor,
        WriteGPUMultiplyColor,
        WriteGPUActiveBlending,
        WriteGPUSelectedTexture,
        WriteGPUSelectedRegion,
        WriteGPUDrawingPointX,
        WriteGPUDrawingPointY,
        WriteGPUDrawingScaleX,
        WriteGPUDrawingScaleY,
        WriteGPUDrawingAngle,
        WriteGPURegionMinX,
        WriteGPURegionMinY,
        WriteGPURegionMaxX,
        WriteGPURegionMaxY,
        WriteGPURegionHotspotX,
        WriteGPURegionHotspotY
    };
    
    
    // =============================================================================
    //      V32 GPU: INSTANCE HANDLING
    // =============================================================================
    
    
    V32GPU::V32GPU()
    {
        // no entities were pointed yet
        PointedTexture = nullptr;
        PointedRegion = nullptr;
        
        // size the array
        CartridgeTextures.resize( Constants::GPUMaximumCartridgeTextures );
        
        // no cartridge loaded yet
        LoadedCartridgeTextures = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    V32GPU::~V32GPU()
    {
        // don't release any textures
        // (this is done at console destructor)
    }
    
    
    // =============================================================================
    //      V32 GPU: HANDLING TEXTURES ON CARTRIDGE EVENTS
    // =============================================================================
    
    
    void V32GPU::InsertCartridgeTextures( uint32_t NumberOfCartridgeTextures )
    {
        if( NumberOfCartridgeTextures > Constants::GPUMaximumCartridgeTextures )
          Callbacks::ThrowException( "Attempting to insert too many cartridge textures" );
        
        LoadedCartridgeTextures = NumberOfCartridgeTextures;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32GPU::RemoveCartridgeTextures()
    {
        LoadedCartridgeTextures = 0;
        Callbacks::UnloadCartridgeTextures();
    }
    
    
    // =============================================================================
    //      V32 GPU: I/O BUS CONNECTION
    // =============================================================================
    
    
    bool V32GPU::ReadPort( int32_t LocalPort, V32Word& Result )
    {
        // check range
        if( LocalPort > GPU_LastPort )
          return false;
        
        // command port is write-only
        if( LocalPort == (int32_t)GPU_LocalPorts::Command )
          return false;
        
        // CASE 1: read from GPU-level parameters
        if( LocalPort < (int32_t)GPU_LocalPorts::RegionMinX )
        {
            V32Word* GPURegisters = (V32Word*)(&Command);
            Result = GPURegisters[ LocalPort ];
        }
        
        // CASE 2: Read from region-level parameters
        else
        {
            V32Word* RegionRegisters = (V32Word*)PointedRegion;
            int32_t RegionPort = LocalPort - (int32_t)GPU_LocalPorts::RegionMinX;
            Result = RegionRegisters[ RegionPort ];
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32GPU::WritePort( int32_t LocalPort, V32Word Value )
    {
        // check range
        if( LocalPort > GPU_LastPort )
          return false;
        
        // redirect to the needed specific writer
        return GPUPortWriterTable[ LocalPort ]( *this, Value );
    }
    
    
    // =============================================================================
    //      V32 GPU: GENERAL OPERATION
    // =============================================================================
    
    
    void V32GPU::ChangeFrame()
    {
        // restore the drawing capacity for next frame
        RemainingPixels = Constants::GPUPixelCapacityPerFrame;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32GPU::Reset()
    {
        // reset all global ports to default values
        Command = 0;
        RemainingPixels = Constants::GPUPixelCapacityPerFrame;
        
        // registers: draw command parameters
        DrawingPointX = 0;
        DrawingPointY = 0;
        DrawingScaleX = 1.0f;
        DrawingScaleY = 1.0f;
        DrawingAngle  = 0.0f; 
        
        // registers: global graphic parameters
        ClearColor     = {   0,   0,   0, 255 };
        MultiplyColor  = { 255, 255, 255, 255 };
        ActiveBlending = (int32_t)IOPortValues::GPUBlendingMode_Alpha;
        SelectedTexture = -1;
        SelectedRegion = 0;
        
        // notify video library of parameter changes
        Callbacks::SelectTexture( SelectedTexture );
        Callbacks::SetMultiplyColor( MultiplyColor );
        Callbacks::SetBlendingMode( ActiveBlending );
        
        // reset pointed entities
        PointedTexture = &BiosTexture;
        PointedRegion = &BiosTexture.Regions[ 0 ];
        
        // reset all regions for every textures
        // (but keep all existent textures reloaded!)
        for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
        for( int j = 0; j < Constants::GPURegionsPerTexture; j++ )
        {
            CartridgeTextures[ i ].Regions[ j ].MinX     = 0;
            CartridgeTextures[ i ].Regions[ j ].MinX     = 0;
            CartridgeTextures[ i ].Regions[ j ].MaxX     = 0;
            CartridgeTextures[ i ].Regions[ j ].MaxY     = 0;
            CartridgeTextures[ i ].Regions[ j ].HotspotX = 0;
            CartridgeTextures[ i ].Regions[ j ].HotspotY = 0;
        }
        
        // same for BIOS texture
        for( int j = 0; j < Constants::GPURegionsPerTexture; j++ )
        {
            BiosTexture.Regions[ j ].MinX     = 0;
            BiosTexture.Regions[ j ].MinX     = 0;
            BiosTexture.Regions[ j ].MaxX     = 0;
            BiosTexture.Regions[ j ].MaxY     = 0;
            BiosTexture.Regions[ j ].HotspotX = 0;
            BiosTexture.Regions[ j ].HotspotY = 0;
        }
        
        // initial screen clear to black
        Callbacks::ClearScreen( ClearColor );
    }
    
    
    // =============================================================================
    //      V32 GPU: EXECUTION OF GPU COMMANDS
    // =============================================================================
    
    
    void V32GPU::ClearScreen()
    {
        // auto-reject the operation if the GPU is already out of capacity
        if( RemainingPixels < 0 )
          return;
        
        // calculate the needed capacity for this operation
        float CostFactor = 1 + Constants::GPUClearScreenPenalty;
        int32_t NeededPixels = CostFactor * Constants::ScreenPixels;
        
        // reject this request if it cannot be finished in this frame
        RemainingPixels -= NeededPixels;
        
        if( RemainingPixels < 0 )
        {
            RemainingPixels = -1;
            return;
        }
        
        // clear the screen
        Callbacks::ClearScreen( ClearColor );
    }
    
    // -----------------------------------------------------------------------------
    
    // this same method will service the 4 variants of the
    // draw region command, by varying the enabled transforms
    void V32GPU::DrawRegion( bool ScalingEnabled, bool RotationEnabled )
    {
        // auto-reject the operation if the GPU is already out of capacity
        if( RemainingPixels < 0 )
          return;
        
        // get active region
        GPURegion Region = *PointedRegion;
        
        // precalculate region size
        int32_t RegionWidth  = abs(Region.MaxX - Region.MinX) + 1;
        int32_t RegionHeight = abs(Region.MaxY - Region.MinY) + 1;
        
        // precalculate region render size
        float RenderWidth  = RegionWidth;
        float RenderHeight = RegionHeight;
        
        if( ScalingEnabled )
        {
            RenderWidth  *= abs( DrawingScaleX );
            RenderHeight *= abs( DrawingScaleY );
        }
        
        // calculate the needed capacity for this operation
        int32_t EffectiveWidth  = min( (int)RenderWidth,  Constants::ScreenWidth  );
        int32_t EffectiveHeight = min( (int)RenderHeight, Constants::ScreenHeight );
        
        float CostFactor = 1;
        if( ScalingEnabled  ) CostFactor += Constants::GPUScalingPenalty;
        if( RotationEnabled ) CostFactor += Constants::GPURotationPenalty;
        
        int32_t NeededPixels = CostFactor * EffectiveWidth * EffectiveHeight;
        
        // reject this request if it cannot be finished in this frame
        RemainingPixels -= NeededPixels;
        
        if( RemainingPixels < 0 )
        {
            RemainingPixels = -1;
            return;
        }
        
        // calculate absolute texture coordinates
        // (initially, they are pixel-centered and uncorrected)
        float TextureMinX = Region.MinX + 0.5;
        float TextureMaxX = Region.MaxX + 0.5;
        float TextureMinY = Region.MinY + 0.5;
        float TextureMaxY = Region.MaxY + 0.5;
        
        // for large scalings, adjust intra-pixel coordinates
        // to get a more precise sampling when zooming; otherwise
        // the size of pixels will be incorrect at boundaries
        if( ScalingEnabled )
        {
            // check the need of resampling along X and Y
            if( abs( DrawingScaleX ) > 1.0 )
            {
                float PixelCorrectionX = 0.5 - 1.0 / (2.0 * fabs( DrawingScaleX ));
                
                // we allow RegionMinX > RegionMaxX (mirror effect on X)
                // but in that case we need to mirror our sampling correction
                if( TextureMinX < TextureMaxX )
                {
                    TextureMinX -= PixelCorrectionX;
                    TextureMaxX += PixelCorrectionX;
                }
                else
                {
                    TextureMaxX -= PixelCorrectionX;
                    TextureMinX += PixelCorrectionX;
                }
            }
            
            // check the need of resampling along X and Y
            if( abs( DrawingScaleY ) > 1.0 )
            {
                float PixelCorrectionY = 0.5 - 1.0 / (2.0 * fabs( DrawingScaleY ));
                
                // we allow RegionMinY > RegionMaxY (mirror effect on Y)
                // but in that case we need to mirror our sampling correction
                if( TextureMinY < TextureMaxY )
                {
                    TextureMinY -= PixelCorrectionY;
                    TextureMaxY += PixelCorrectionY;
                }
                else
                {
                    TextureMaxY -= PixelCorrectionY;
                    TextureMinY += PixelCorrectionY;
                }
            }
        }
        
        // make texture coordinates relative
        TextureMinX /= Constants::GPUTextureSize;
        TextureMaxX /= Constants::GPUTextureSize;
        TextureMinY /= Constants::GPUTextureSize;
        TextureMaxY /= Constants::GPUTextureSize;
        
        // calculate screen coordinates relative to the hotspot
        // (that way we can use OpenGL transforms to rotate)
        int RelativeMinX = Region.MinX - Region.HotspotX;
        int RelativeMinY = Region.MinY - Region.HotspotY;
        int RelativeMaxX = RelativeMinX + RegionWidth;
        int RelativeMaxY = RelativeMinY + RegionHeight;
        
        // initialize quad vertex positions (in render coordinates)
        RegionQuad.Vertices[ 0 ].x = RelativeMinX;
        RegionQuad.Vertices[ 0 ].y = RelativeMinY;
        RegionQuad.Vertices[ 1 ].x = RelativeMaxX;
        RegionQuad.Vertices[ 1 ].y = RelativeMinY;
        RegionQuad.Vertices[ 2 ].x = RelativeMinX;
        RegionQuad.Vertices[ 2 ].y = RelativeMaxY;
        RegionQuad.Vertices[ 3 ].x = RelativeMaxX;
        RegionQuad.Vertices[ 3 ].y = RelativeMaxY;
        
        // initialize quad texture coordinates (relative to texture: [0-1])
        RegionQuad.Vertices[ 0 ].texture_x = TextureMinX;
        RegionQuad.Vertices[ 0 ].texture_y = TextureMinY;
        RegionQuad.Vertices[ 1 ].texture_x = TextureMaxX;
        RegionQuad.Vertices[ 1 ].texture_y = TextureMinY;
        RegionQuad.Vertices[ 2 ].texture_x = TextureMinX;
        RegionQuad.Vertices[ 2 ].texture_y = TextureMaxY;
        RegionQuad.Vertices[ 3 ].texture_x = TextureMaxX;
        RegionQuad.Vertices[ 3 ].texture_y = TextureMaxY;
        
        // precalculate angle properties when needed
        float AngleCos, AngleSin;
        
        if( RotationEnabled )
        {
            AngleCos = cos( DrawingAngle );
            AngleSin = sin( DrawingAngle );
        }
        
        // apply 2D transforms to the quad
        for( int i = 0; i < 4; i++ )
        {
            float* VertexX = &RegionQuad.Vertices[ i ].x;
            float* VertexY = &RegionQuad.Vertices[ i ].y;
            
            // transform 1: apply scaling
            if( ScalingEnabled )
            {
                *VertexX *= DrawingScaleX;
                *VertexY *= DrawingScaleY;        
            }
            
            // transform 2: apply rotation
            if( RotationEnabled )
            {
                float CopiedX = *VertexX;
                float CopiedY = *VertexY;
                
                *VertexX = CopiedX * AngleCos - CopiedY * AngleSin;
                *VertexY = CopiedX * AngleSin + CopiedY * AngleCos;
            }
            
            // transform 3: apply translation
            *VertexX += DrawingPointX;
            *VertexY += DrawingPointY;
            
            // for some reason negative scaling displaces images
            // by 1 pixel, so fix it after all transforms
            if( ScalingEnabled )
            {
                if( DrawingScaleX < 0 ) *VertexX += 1;
                if( DrawingScaleY < 0 ) *VertexY += 1;
            }
        }
        
        // draw rectangle defined as a quad (4-vertex polygon)
        Callbacks::SelectTexture( SelectedTexture );
        Callbacks::DrawQuad( RegionQuad );
    }
}
