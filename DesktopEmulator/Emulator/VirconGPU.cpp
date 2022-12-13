// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "VirconGPU.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PORT WRITERS TABLE
// =============================================================================


typedef void (*GPUPortWriter)( VirconGPU&, VirconWord );

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
//      VIRCON GPU: INSTANCE HANDLING
// =============================================================================


VirconGPU::VirconGPU()
{
    PointedTexture = nullptr;
    PointedRegion = nullptr;
    
    BiosTexture.TextureID = 0;
}

// -----------------------------------------------------------------------------

VirconGPU::~VirconGPU()
{
    // release BIOS texture
    UnloadTexture( BiosTexture );
    
    // release all cartridge textures
    for( GPUTexture& T: CartridgeTextures )
      UnloadTexture( T );
      
    CartridgeTextures.clear();
}


// =============================================================================
//      VIRCON GPU: HANDLING VIDEO RESOURCES
// =============================================================================


void VirconGPU::LoadTexture( GPUTexture& TargetTexture, void* Pixels, unsigned Width, unsigned Height )
{
    // check for page number limit
    if( (int)CartridgeTextures.size() >= Constants::GPUMaximumCartridgeTextures )
      THROW( "All available GPU textures are already loaded" );
    
    // check for size limits
    if( (int)Width > Constants::GPUTextureSize || (int)Height > Constants::GPUTextureSize )
      THROW( "Loaded image is too large to fit in a GPU texture" );
    
    // create a new OpenGL texture and select it
    GLuint TextureID;
    glGenTextures( 1, &TextureID );
    glBindTexture( GL_TEXTURE_2D, TextureID );
    
    // check correct texture ID
    if( !TextureID )
      THROW( "OpenGL failed to generate a new texture" );
    
    // clear OpenGL errors
    glGetError();
    
    // (1) first we build an empty texture of the extented size
    glTexImage2D
    (
        GL_TEXTURE_2D,              // texture is a 2D rectangle
        0,                          // level of detail (0 = normal size)
        GL_RGBA,                    // color components in the texture
        Constants::GPUTextureSize,  // texture width in pixels
        Constants::GPUTextureSize,  // texture height in pixels
        0,                          // border width (must be 0 or 1)
        GL_RGBA,                    // color components in the source
        GL_UNSIGNED_BYTE,           // each color component is a byte
        nullptr                     // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not create an empty OpenGL texture" );
    
    // (2) then we modify the part of our image
    glTexSubImage2D
    (
        GL_TEXTURE_2D,       // texture is a 2D rectangle
        0,                   // level of detail (0 = normal size)
        0,                   // x offset
        0,                   // y offset
        Width,               // image width in pixels
        Height,              // image height in pixels
        GL_RGBA,             // color components in the source
        GL_UNSIGNED_BYTE,    // each color component is a byte
        Pixels               // buffer storing the texture data
    );
    
    // check correct conversion
    if( glGetError() != GL_NO_ERROR )
      THROW( "Could not copy the loaded SDL image to the OpenGL texture" );
    
    // textures must be scaled using only nearest neighbour
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );         
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    // out-of-texture coordinates must clamp, not wrap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    // finally assign the OpenGL ID to target GPU texture
    TargetTexture.TextureID = TextureID;
}

// -----------------------------------------------------------------------------

void VirconGPU::UnloadTexture( GPUTexture& TargetTexture )
{
    if( TargetTexture.TextureID == 0 )
      return;
    
    glDeleteTextures( 1, &TargetTexture.TextureID );
    TargetTexture.TextureID = 0;
}


// =============================================================================
//      VIRCON GPU: I/O BUS CONNECTION
// =============================================================================


bool VirconGPU::ReadPort( int32_t LocalPort, VirconWord& Result )
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
        VirconWord* GPURegisters = (VirconWord*)(&Command);
        Result = GPURegisters[ LocalPort ];
    }
    
    // CASE 2: Read from region-level parameters
    else
    {
        VirconWord* RegionRegisters = (VirconWord*)PointedRegion;
        int32_t RegionPort = LocalPort - (int32_t)GPU_LocalPorts::RegionMinX;
        Result = RegionRegisters[ RegionPort ];
    }
    
    return true;
}

// -----------------------------------------------------------------------------

bool VirconGPU::WritePort( int32_t LocalPort, VirconWord Value )
{
    // check range
    if( LocalPort > GPU_LastPort )
      return false;
    
    // redirect to the needed specific writer
    GPUPortWriterTable[ LocalPort ]( *this, Value );
    return true;
}


// =============================================================================
//      VIRCON GPU: GENERAL OPERATION
// =============================================================================


void VirconGPU::ChangeFrame()
{
    // restore the drawing capacity for next frame
    RemainingPixels = Constants::GPUPixelCapacityPerFrame;
}

// -----------------------------------------------------------------------------

void VirconGPU::Reset()
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
    ClearColor     = {   0,   0,   0,   0 };
    MultiplyColor  = { 255, 255, 255, 255 };
    ActiveBlending = (int32_t)IOPortValues::GPUBlendingMode_Alpha;
    SelectedTexture = -1;
    SelectedRegion = 0;
    
    // reset pointed entities
    PointedTexture = &BiosTexture;
    PointedRegion = &BiosTexture.Regions[ 0 ];
    
    // reset all regions for every textures
    // (but keep all existent textures reloaded!)
    for( GPUTexture& Texture: CartridgeTextures )
    for( int i = 0; i < Constants::GPURegionsPerTexture; i++ )
    {
        Texture.Regions[ i ].MinX     = 0;
        Texture.Regions[ i ].MinX     = 0;
        Texture.Regions[ i ].MaxX     = 0;
        Texture.Regions[ i ].MaxY     = 0;
        Texture.Regions[ i ].HotspotX = 0;
        Texture.Regions[ i ].HotspotY = 0;
    }
    
    // same for BIOS texture
    for( int i = 0; i < Constants::GPURegionsPerTexture; i++ )
    {
        BiosTexture.Regions[ i ].MinX     = 0;
        BiosTexture.Regions[ i ].MinX     = 0;
        BiosTexture.Regions[ i ].MaxX     = 0;
        BiosTexture.Regions[ i ].MaxY     = 0;
        BiosTexture.Regions[ i ].HotspotX = 0;
        BiosTexture.Regions[ i ].HotspotY = 0;
    }
    
    // initial graphic settings
    OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    OpenGL2D.SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    
    // clear the screen
    OpenGL2D.RenderToFramebuffer();
    OpenGL2D.ClearScreen( GPUColor{ 0, 0, 0, 255 } );
}


// =============================================================================
//      VIRCON GPU: EXECUTION OF GPU COMMANDS
// =============================================================================


void VirconGPU::ClearScreen()
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
    OpenGL2D.ClearScreen( ClearColor );
}

// -----------------------------------------------------------------------------

// this same method will service the 4 variants of the
// draw region command, by varying the enabled transforms
void VirconGPU::DrawRegion( bool ScalingEnabled, bool RotationEnabled )
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
    
    // select this texture
    glBindTexture( GL_TEXTURE_2D, PointedTexture->TextureID );
    glEnable( GL_TEXTURE_2D );
    
    // calculate relative texture coordinates
    float TextureMinX = (Region.MinX+0.5) / Constants::GPUTextureSize;
    float TextureMinY = (Region.MinY+0.5) / Constants::GPUTextureSize;
    float TextureMaxX = (Region.MaxX+0.5) / Constants::GPUTextureSize;
    float TextureMaxY = (Region.MaxY+0.5) / Constants::GPUTextureSize;
    
    // calculate screen coordinates relative to the hotspot
    // (that way we can use OpenGL transforms to rotate)
    int RelativeMinX = Region.MinX - Region.HotspotX;
    int RelativeMinY = Region.MinY - Region.HotspotY;
    int RelativeMaxX = RelativeMinX + RegionWidth;
    int RelativeMaxY = RelativeMinY + RegionHeight;
    
    // for some reason negative scaling displaces images 1 pixel
    int TranslationX = DrawingPointX;
    int TranslationY = DrawingPointY;
    
    if( ScalingEnabled )
    {
        if( DrawingScaleX < 0 ) TranslationX += 1;
        if( DrawingScaleY < 0 ) TranslationY += 1;
    }
    
    // set vertex positions (in render coordinates)
    OpenGL2D.SetQuadVertexPosition( 0, RelativeMinX, RelativeMinY );
    OpenGL2D.SetQuadVertexPosition( 1, RelativeMaxX, RelativeMinY );
    OpenGL2D.SetQuadVertexPosition( 2, RelativeMinX, RelativeMaxY );
    OpenGL2D.SetQuadVertexPosition( 3, RelativeMaxX, RelativeMaxY );
    
    // and texture coordinates (relative to texture: [0-1])
    OpenGL2D.SetQuadVertexTexCoords( 0, TextureMinX, TextureMinY );
    OpenGL2D.SetQuadVertexTexCoords( 1, TextureMaxX, TextureMinY );
    OpenGL2D.SetQuadVertexTexCoords( 2, TextureMinX, TextureMaxY );
    OpenGL2D.SetQuadVertexTexCoords( 3, TextureMaxX, TextureMaxY );
    
    // prepare the needed 2D spatial transforms
    OpenGL2D.SetTranslation( TranslationX, TranslationY );
    
    if( ScalingEnabled )
      OpenGL2D.SetScale( DrawingScaleX, DrawingScaleY );
    
    if( RotationEnabled )
      OpenGL2D.SetRotation( DrawingAngle );
    
    OpenGL2D.ComposeTransform( ScalingEnabled, RotationEnabled );
    
    // draw rectangle defined as a quad (4-vertex polygon)
    OpenGL2D.DrawTexturedQuad();
}
