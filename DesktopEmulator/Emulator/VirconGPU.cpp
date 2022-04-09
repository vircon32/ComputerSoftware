// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDefinitions.hpp"
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/OpenGL2D.hpp"
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


void VirconGPU::LoadTexture( GPUTexture& TargetTexture, const string& FilePath )
{
    // open the file
    LOG( "Loading GPU texture from RGBA file \"" << FilePath << "\"" );
    
    ifstream InputFile;
    InputFile.open( FilePath, ios::binary | ios::ate );
    
    if( InputFile.fail() )
      THROW( "Cannot open RGBA file" );
    
    // obtain file size
    unsigned NumberOfBytes = InputFile.tellg();
    
    // obtain image dimensions
    uint32_t Width, Height;
    InputFile.seekg( 0, ios::beg );
    InputFile.read( (char*)(&Width ), 4 );
    InputFile.read( (char*)(&Height), 4 );

    // check size coherency
    if( NumberOfBytes != (Width*Height*4 + 8) )
    {
        InputFile.close();
        THROW( "Size of RGBA file does not match the specified dimensions of " + to_string(Width) + " x " + to_string(Height) + " pixels" );
    }
    
    // load the whole RGBA data to a buffer
    vector< uint8_t > TextureBytes;
    TextureBytes.resize( NumberOfBytes );
    InputFile.read( (char*)(&TextureBytes[0]), NumberOfBytes - 8 );
    
    // we can now close the input file
    InputFile.close();
    
    // now, just call the other loader function with this buffer
    LoadTexture( TargetTexture, &TextureBytes[ 0 ], Width, Height );
}

// -----------------------------------------------------------------------------

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
        GL_RGBA8,                   // color components
        Constants::GPUTextureSize,  // texture width in pixels
        Constants::GPUTextureSize,  // texture height in pixels
        0,                          // border width (must be 0 or 1)
        GL_RGBA,                    // buffer format for color components
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
        GL_RGBA,             // buffer format for color components
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
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    
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
    SetBlendingMode( BlendingMode::Alpha );
    glClearColor( 0,0,0,1 );
    glColor4ub( 255,255,255,255 );
    
    // clear the screen
    OpenGL2D.RenderToFramebuffer();
    glClear( GL_COLOR_BUFFER_BIT );
}


// =============================================================================
//      VIRCON GPU: EXECUTION OF GPU COMMANDS
// =============================================================================


void VirconGPU::ClearScreen()
{
    // reject this request if it cannot be finished in this frame
    float CostFactor = 1 + Constants::GPUClearScreenPenalty;
    int32_t NeededPixels = CostFactor * Constants::ScreenPixels;
    
    // reject this request if it cannot be finished in this frame
    RemainingPixels -= NeededPixels;
    
    if( RemainingPixels < 0 )
    {
        RemainingPixels = 0;
        return;
    }
    
    // take precautions to ensure a correct clearing
    glDisable( GL_TEXTURE_2D );
    
    glColor4ub
    (
        ClearColor.R,
        ClearColor.G,
        ClearColor.B,
        ClearColor.A
    );
    
    // perform the screen clear by drawing a full-screen quad
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glVertex2f(                      0,                       0 );
        glVertex2f( Constants::ScreenWidth,                       0 );
        glVertex2f( Constants::ScreenWidth, Constants::ScreenHeight );
        glVertex2f(                      0, Constants::ScreenHeight );
    }
    glEnd();    
    
    // restore previous state
    glEnable( GL_TEXTURE_2D );
    
    glColor4ub
    (
        MultiplyColor.R,
        MultiplyColor.G,
        MultiplyColor.B,
        MultiplyColor.A
    );
}

// -----------------------------------------------------------------------------

void VirconGPU::DrawRegion()
{
    // get active region
    GPURegion Region = *PointedRegion;
    
    // precalculate region size
    int32_t RegionWidth  = abs(Region.MaxX - Region.MinX) + 1;
    int32_t RegionHeight = abs(Region.MaxY - Region.MinY) + 1;
    
    // calculate the needed capacity for this operation
    int32_t EffectiveWidth  = min( RegionWidth,  Constants::ScreenWidth  );
    int32_t EffectiveHeight = min( RegionHeight, Constants::ScreenHeight );
    float CostFactor = 1;
    int32_t NeededPixels = CostFactor * EffectiveWidth * EffectiveHeight;
    
    // reject this request if it cannot be finished in this frame
    RemainingPixels -= NeededPixels;
    
    if( RemainingPixels < 0 )
    {
        RemainingPixels = 0;
        return;
    }
    
    // select this texture
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, PointedTexture->TextureID );
    
    // calculate relative texture coordinates
    float TextureMinX = (Region.MinX+0.5) / Constants::GPUTextureSize;
    float TextureMinY = (Region.MinY+0.5) / Constants::GPUTextureSize;
    float TextureMaxX = (Region.MaxX+0.5) / Constants::GPUTextureSize;
    float TextureMaxY = (Region.MaxY+0.5) / Constants::GPUTextureSize;
    
    // precalculate screen coordinates
    int RenderMinX = DrawingPointX + (Region.MinX - Region.HotspotX);
    int RenderMinY = DrawingPointY + (Region.MinY - Region.HotspotY);
    int RenderMaxX = RenderMinX + RegionWidth;
    int RenderMaxY = RenderMinY + RegionHeight;
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureMinX, TextureMinY );  glVertex2i( RenderMinX, RenderMinY );
        glTexCoord2f( TextureMaxX, TextureMinY );  glVertex2i( RenderMaxX, RenderMinY );
        glTexCoord2f( TextureMaxX, TextureMaxY );  glVertex2i( RenderMaxX, RenderMaxY );
        glTexCoord2f( TextureMinX, TextureMaxY );  glVertex2i( RenderMinX, RenderMaxY );
    }
    glEnd();
}

// -----------------------------------------------------------------------------

void VirconGPU::DrawRegionZoomed()
{
    // get active region
    GPURegion Region = *PointedRegion;
    
    // precalculate region size
    int32_t RegionWidth  = abs(Region.MaxX - Region.MinX) + 1;
    int32_t RegionHeight = abs(Region.MaxY - Region.MinY) + 1;
    
    // precalculate region render size
    float RenderWidth  = DrawingScaleX * RegionWidth;
    float RenderHeight = DrawingScaleY * RegionHeight;
    
    // calculate the needed capacity for this operation
    int32_t EffectiveWidth  = min( (int)RenderWidth,  Constants::ScreenWidth  );
    int32_t EffectiveHeight = min( (int)RenderHeight, Constants::ScreenHeight );
    float CostFactor = 1 + Constants::GPUScalingPenalty;
    int32_t NeededPixels = CostFactor * EffectiveWidth * EffectiveHeight;
    
    // reject this request if it cannot be finished in this frame
    RemainingPixels -= NeededPixels;
    
    if( RemainingPixels < 0 )
    {
        RemainingPixels = 0;
        return;
    }
    
    // select this texture
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, PointedTexture->TextureID );
    
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
    int TranslationX = DrawingPointX + (DrawingScaleX < 0? 1 : 0);
    int TranslationY = DrawingPointY + (DrawingScaleY < 0? 1 : 0);
    
    // apply transform for rendering
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glTranslatef( TranslationX, TranslationY, 0 );
    glScalef( DrawingScaleX, DrawingScaleY, 0 );
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureMinX, TextureMinY );  glVertex2i( RelativeMinX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMinY );  glVertex2i( RelativeMaxX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMaxY );  glVertex2i( RelativeMaxX, RelativeMaxY );
        glTexCoord2f( TextureMinX, TextureMaxY );  glVertex2i( RelativeMinX, RelativeMaxY );
    }
    glEnd();
    
    // undo the previous transform
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

// -----------------------------------------------------------------------------

void VirconGPU::DrawRegionRotated()
{
    // get active region
    GPURegion Region = *PointedRegion;
    
    // precalculate region size
    int32_t RegionWidth  = abs(Region.MaxX - Region.MinX) + 1;
    int32_t RegionHeight = abs(Region.MaxY - Region.MinY) + 1;
    
    // calculate the needed capacity for this operation
    int32_t EffectiveWidth  = min( (int)RegionWidth,  Constants::ScreenWidth  );
    int32_t EffectiveHeight = min( (int)RegionHeight, Constants::ScreenHeight );
    float CostFactor = 1 + Constants::GPURotationPenalty;
    int32_t NeededPixels = CostFactor * EffectiveWidth * EffectiveHeight;
    
    // reject this request if it cannot be finished in this frame
    RemainingPixels -= NeededPixels;
    
    if( RemainingPixels < 0 )
    {
        RemainingPixels = 0;
        return;
    }
    
    // select this texture
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, PointedTexture->TextureID );
    
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
    
    // apply transform for rendering
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glTranslatef( DrawingPointX, DrawingPointY, 0 );
    glRotatef( DrawingAngle * DegreesPerRadian, 0, 0, 1 );
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureMinX, TextureMinY );  glVertex2i( RelativeMinX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMinY );  glVertex2i( RelativeMaxX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMaxY );  glVertex2i( RelativeMaxX, RelativeMaxY );
        glTexCoord2f( TextureMinX, TextureMaxY );  glVertex2i( RelativeMinX, RelativeMaxY );
    }
    glEnd();
    
    // undo the previous transform
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

// -----------------------------------------------------------------------------

void VirconGPU::DrawRegionRotozoomed()
{
    // get active region
    GPURegion Region = *PointedRegion;
    
    // precalculate region size
    int32_t RegionWidth  = abs(Region.MaxX - Region.MinX) + 1;
    int32_t RegionHeight = abs(Region.MaxY - Region.MinY) + 1;
    
    // precalculate region render size
    float RenderWidth  = DrawingScaleX * RegionWidth;
    float RenderHeight = DrawingScaleY * RegionHeight;
    
    // calculate the needed capacity for this operation
    int32_t EffectiveWidth  = min( (int)RenderWidth,  Constants::ScreenWidth  );
    int32_t EffectiveHeight = min( (int)RenderHeight, Constants::ScreenHeight );
    float CostFactor = 1 + Constants::GPUScalingPenalty + Constants::GPURotationPenalty;
    int32_t NeededPixels = CostFactor * EffectiveWidth * EffectiveHeight;
    
    // reject this request if it cannot be finished in this frame
    RemainingPixels -= NeededPixels;
    
    if( RemainingPixels < 0 )
    {
        RemainingPixels = 0;
        return;
    }
    
    // select this texture
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, PointedTexture->TextureID );
    
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
    int TranslationX = DrawingPointX + (DrawingScaleX < 0? 1 : 0);
    int TranslationY = DrawingPointY + (DrawingScaleY < 0? 1 : 0);
    
    // apply transform for rendering
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glTranslatef( TranslationX, TranslationY, 0 );
    glScalef( DrawingScaleX, DrawingScaleY, 0 );
    glRotatef( DrawingAngle * DegreesPerRadian, 0, 0, 1 );
    
    // draw a rectangle defined as a quad (4-vertex polygon)
    glBegin( GL_QUADS );
    {
        // set pairs of point position (in render coordinates)
        // and texture coordinates (relative to texture: [0-1])
        glTexCoord2f( TextureMinX, TextureMinY );  glVertex2i( RelativeMinX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMinY );  glVertex2i( RelativeMaxX, RelativeMinY );
        glTexCoord2f( TextureMaxX, TextureMaxY );  glVertex2i( RelativeMaxX, RelativeMaxY );
        glTexCoord2f( TextureMinX, TextureMaxY );  glVertex2i( RelativeMinX, RelativeMaxY );
    }
    glEnd();
    
    // undo the previous transform
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}
