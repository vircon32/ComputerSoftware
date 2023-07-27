// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include project headers
    #include "V32GPU.hpp"
    #include "AuxiliaryFunctions.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Mathematics
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      PORT WRITE FUNCTIONS FOR V32 GPU
    // =============================================================================
    
    
    bool WriteGPUCommand( V32GPU& GPU, V32Word Value )
    {
        // now execute the command, if valid
        switch( Value.AsInteger )
        {
            case (int32_t)IOPortValues::GPUCommand_ClearScreen:
                GPU.ClearScreen();
                break;
                
            case (int32_t)IOPortValues::GPUCommand_DrawRegion:
                GPU.DrawRegion( false, false );
                break;
                
            case (int32_t)IOPortValues::GPUCommand_DrawRegionZoomed:
                GPU.DrawRegion( true, false );
                break;
                
            case (int32_t)IOPortValues::GPUCommand_DrawRegionRotated:
                GPU.DrawRegion( false, true );
                break;
                
            case (int32_t)IOPortValues::GPUCommand_DrawRegionRotozoomed:
                GPU.DrawRegion( true, true );
                break;
                
            // unknown command codes are ignored, but
            // they don't trigger a port write error
            default: break;
        }
        
        // do not write the value;
        // it is useless anyway (this port is write-only)
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURemainingPixels( V32GPU& GPU, V32Word Value )
    {
        // reject request: this register is read-only
        return false;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUClearColor( V32GPU& GPU, V32Word Value )
    {
        // just write the value
        GPU.ClearColor = Value.AsColor;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUMultiplyColor( V32GPU& GPU, V32Word Value )
    {
        // first write the value
        GPU.MultiplyColor = Value.AsColor;
        
        // notify the video library
        Callbacks::SetMultiplyColor( Value.AsColor );
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUActiveBlending( V32GPU& GPU, V32Word Value )
    {
        switch( Value.AsInteger )
        {
            case (int32_t)IOPortValues::GPUBlendingMode_Alpha:
            {
                GPU.ActiveBlending = Value.AsInteger;
                break;
            }
            
            case (int32_t)IOPortValues::GPUBlendingMode_Add:
            {
                GPU.ActiveBlending = Value.AsInteger;
                break;
            }
            
            case (int32_t)IOPortValues::GPUBlendingMode_Subtract:
            {
                GPU.ActiveBlending = Value.AsInteger;
                break;
            }
            
            // unknown blending mode codes are just ignored
            // (the value is not written either)
            return true;
        }
        
        // for valid modes, notify the video library
        Callbacks::SetBlendingMode( Value.AsInteger );
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUSelectedTexture( V32GPU& GPU, V32Word Value )
    {
        // prevent setting a non-existent texture
        if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)GPU.CartridgeTextures.size() )
          return true;
            
        // write the value
        GPU.SelectedTexture = Value.AsInteger;
        
        // notify the video library
        Callbacks::SelectTexture( Value.AsInteger );
        
        // now update the pointed entities
        if( Value.AsInteger == -1 )
        {
            // special case for BIOS texture
            GPU.PointedTexture = &GPU.BiosTexture;
            GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
        }
        else
        {
            // regular cartridge textures
            GPU.PointedTexture = &GPU.CartridgeTextures[ GPU.SelectedTexture ];
            GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
        }
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUSelectedRegion( V32GPU& GPU, V32Word Value )
    {
        // prevent setting a non-existent region
        if( Value.AsInteger < 0 || Value.AsInteger >= Constants::GPURegionsPerTexture )
          return true;
        
        // write the value
        GPU.SelectedRegion = Value.AsInteger;
        
        // update pointed entity
        GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUDrawingPointX( V32GPU& GPU, V32Word Value )
    {
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsInteger, -1000, Constants::ScreenWidth + 1000 );
        GPU.DrawingPointX = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUDrawingPointY( V32GPU& GPU, V32Word Value )
    {
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsInteger, -1000, Constants::ScreenHeight + 1000 );
        GPU.DrawingPointY = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUDrawingScaleX( V32GPU& GPU, V32Word Value )
    {
        // ignore non-numeric values
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, -1024, 1024 );
        GPU.DrawingScaleX = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUDrawingScaleY( V32GPU& GPU, V32Word Value )
    {
        // ignore non-numeric values
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, -1024, 1024 );
        GPU.DrawingScaleY = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPUDrawingAngle( V32GPU& GPU, V32Word Value )
    {
        // ignore non-numeric values
        if( isnan( Value.AsFloat ) || isinf( Value.AsFloat ) )
          return true;
        
        // out of range values are accepted, but they are clamped
        Clamp( Value.AsFloat, -1024, 1024 );
        GPU.DrawingAngle = Value.AsFloat;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionMinX( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are accepted,
        // but they are clamped to texture limits
        Clamp( Value.AsInteger, 0, Constants::GPUTextureSize-1 );
        GPU.PointedRegion->MinX = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionMinY( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are accepted,
        // but they are clamped to texture limits
        Clamp( Value.AsInteger, 0, Constants::GPUTextureSize-1 );
        GPU.PointedRegion->MinY = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionMaxX( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are accepted,
        // but they are clamped to texture limits
        int32_t ValidX = Value.AsInteger;
        Clamp( ValidX, 0, Constants::GPUTextureSize-1 );
        
        GPU.PointedRegion->MaxX = ValidX;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionMaxY( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are accepted,
        // but they are clamped to texture limits
        Clamp( Value.AsInteger, 0, Constants::GPUTextureSize-1 );
        GPU.PointedRegion->MaxY = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionHotspotX( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are valid up to
        // a certain range, then they get clamped
        Clamp( Value.AsInteger, -Constants::GPUTextureSize, (2*Constants::GPUTextureSize)-1 );
        GPU.PointedRegion->HotspotX = Value.AsInteger;
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool WriteGPURegionHotspotY( V32GPU& GPU, V32Word Value )
    {
        // out of texture values are valid
        Clamp( Value.AsInteger, -Constants::GPUTextureSize, (2*Constants::GPUTextureSize)-1 );
        GPU.PointedRegion->HotspotY = Value.AsInteger;
        return true;
    }
}
