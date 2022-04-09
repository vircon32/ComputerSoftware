// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include project headers
    #include "VirconGPU.hpp"
    
    // include C/C++ headers
    #include <cmath>            // [ ANSI C ] Mathematics
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


void WriteGPUCommand( VirconGPU& GPU, VirconWord Value )
{
    // first write the value
    GPU.Command = Value.AsInteger;
    
    // now execute the command, if valid
    switch( Value.AsInteger )
    {
        case (int32_t)IOPortValues::GPUCommand_ClearScreen:
            GPU.ClearScreen();
            return;
            
        case (int32_t)IOPortValues::GPUCommand_DrawRegion:
            GPU.DrawRegion();
            return;
            
        case (int32_t)IOPortValues::GPUCommand_DrawRegionZoomed:
            GPU.DrawRegionZoomed();
            return;
            
        case (int32_t)IOPortValues::GPUCommand_DrawRegionRotated:
            GPU.DrawRegionRotated();
            return;
            
        case (int32_t)IOPortValues::GPUCommand_DrawRegionRotozoomed:
            GPU.DrawRegionRotozoomed();
            return;
            
        // (unknown command codes are just ignored)
        default: return;
    }
}

// -----------------------------------------------------------------------------

void WriteGPURemainingPixels( VirconGPU& GPU, VirconWord Value )
{
    // (ignore request: this register is read-only)
}

// -----------------------------------------------------------------------------

void WriteGPUClearColor( VirconGPU& GPU, VirconWord Value )
{
    // just write the value
    GPU.ClearColor = Value.AsColor;
}

// -----------------------------------------------------------------------------

void WriteGPUMultiplyColor( VirconGPU& GPU, VirconWord Value )
{
    // first write the value
    GPU.MultiplyColor = Value.AsColor;
    
    // now update the corresponding value in OpenGL
    glColor4ub
    (
        Value.AsColor.R,
        Value.AsColor.G,
        Value.AsColor.B,
        Value.AsColor.A
    );
}

// -----------------------------------------------------------------------------

void WriteGPUActiveBlending( VirconGPU& GPU, VirconWord Value )
{
    switch( Value.AsInteger )
    {
        case (int32_t)IOPortValues::GPUBlendingMode_Alpha:
        {
            SetBlendingMode( BlendingMode::Alpha );
            GPU.ActiveBlending = Value.AsInteger;
            break;
        }
        
        case (int32_t)IOPortValues::GPUBlendingMode_Add:
        {
            SetBlendingMode( BlendingMode::Add );
            GPU.ActiveBlending = Value.AsInteger;
            break;
        }
        
        case (int32_t)IOPortValues::GPUBlendingMode_Subtract:
        {
            SetBlendingMode( BlendingMode::Subtract );
            GPU.ActiveBlending = Value.AsInteger;
            break;
        }
    }
    
    // unknown blending mode codes are just ignored
    // (the value is not written either)
}

// -----------------------------------------------------------------------------

void WriteGPUSelectedTexture( VirconGPU& GPU, VirconWord Value )
{
    // prevent setting a non-existent texture
    if( Value.AsInteger < -1 || Value.AsInteger >= (int32_t)GPU.CartridgeTextures.size() )
      return;
        
    // write the value
    GPU.SelectedTexture = Value.AsInteger;
    
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
}

// -----------------------------------------------------------------------------

void WriteGPUSelectedRegion( VirconGPU& GPU, VirconWord Value )
{
    // prevent setting a non-existent region
    if( Value.AsInteger < 0 || Value.AsInteger >= Constants::GPURegionsPerTexture )
      return;
    
    // write the value
    GPU.SelectedRegion = Value.AsInteger;
    
    // update pointed entity
    GPU.PointedRegion = &GPU.PointedTexture->Regions[ GPU.SelectedRegion ];
}

// -----------------------------------------------------------------------------

void WriteGPUDrawingPointX( VirconGPU& GPU, VirconWord Value )
{
    // just write it: any value is considered valid
    GPU.DrawingPointX = Value.AsInteger;
}

// -----------------------------------------------------------------------------

void WriteGPUDrawingPointY( VirconGPU& GPU, VirconWord Value )
{
    // just write it: any value is considered valid
    GPU.DrawingPointY = Value.AsInteger;
}

// -----------------------------------------------------------------------------

void WriteGPUDrawingScaleX( VirconGPU& GPU, VirconWord Value )
{
    // Float parameters are only written if they are valid
    // numeric values (otherwise the request is ignored).
    // Negative values are valid, they just mirror coordinates
    if( !isnan( Value.AsFloat ) && !isinf( Value.AsFloat ) )
      GPU.DrawingScaleX = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteGPUDrawingScaleY( VirconGPU& GPU, VirconWord Value )
{
    // Float parameters are only written if they are valid
    // numeric values (otherwise the request is ignored).
    // Negative values are valid, they just mirror coordinates
    if( !isnan( Value.AsFloat ) && !isinf( Value.AsFloat ) )
      GPU.DrawingScaleY = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteGPUDrawingAngle( VirconGPU& GPU, VirconWord Value )
{
    // float parameters are only written if they are valid numeric values
    // (otherwise the request is ignored)
    if( !isnan( Value.AsFloat ) && !isinf( Value.AsFloat ) )
      GPU.DrawingAngle = Value.AsFloat;
}

// -----------------------------------------------------------------------------

void WriteGPURegionMinX( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are accepted,
    // but they are clamped to texture limits
    int32_t ValidX = Value.AsInteger;
    Clamp( ValidX, 0, Constants::GPUTextureSize-1 );
    
    GPU.PointedRegion->MinX = ValidX;
}

// -----------------------------------------------------------------------------

void WriteGPURegionMinY( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are accepted,
    // but they are clamped to texture limits
    int32_t ValidY = Value.AsInteger;
    Clamp( ValidY, 0, Constants::GPUTextureSize-1 );
    
    GPU.PointedRegion->MinY = ValidY;
}

// -----------------------------------------------------------------------------

void WriteGPURegionMaxX( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are accepted,
    // but they are clamped to texture limits
    int32_t ValidX = Value.AsInteger;
    Clamp( ValidX, 0, Constants::GPUTextureSize-1 );
    
    GPU.PointedRegion->MaxX = ValidX;
}

// -----------------------------------------------------------------------------

void WriteGPURegionMaxY( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are accepted,
    // but they are clamped to texture limits
    int32_t ValidY = Value.AsInteger;
    Clamp( ValidY, 0, Constants::GPUTextureSize-1 );
    
    GPU.PointedRegion->MaxY = ValidY;
}

// -----------------------------------------------------------------------------

void WriteGPURegionHotspotX( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are valid
    GPU.PointedRegion->HotspotX = Value.AsInteger;
}

// -----------------------------------------------------------------------------

void WriteGPURegionHotspotY( VirconGPU& GPU, VirconWord Value )
{
    // out of texture values are valid
    GPU.PointedRegion->HotspotY = Value.AsInteger;
}
