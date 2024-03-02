// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "EmulatorControl.hpp"
    #include "GamepadsInput.hpp"
    #include "VideoOutput.hpp"
    #include "AudioOutput.hpp"
    #include "Texture.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROGRAM CONFIGURATION
// =============================================================================


// program state
bool GlobalLoopActive;
bool MouseIsOnWindow;
string EmulatorFolder;
string BiosFileName;

// GUI settings
list< string > RecentCartridgePaths;
list< string > RecentMemoryCardPaths;
string LastCartridgeDirectory;
string LastMemoryCardDirectory;


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// instance of the Vircon virtual machine
V32::V32Console Console;

// wrappers for console I/O operation
EmulatorControl Emulator;
VideoOutput Video;
AudioOutput Audio;
GamepadsInput Gamepads;

// video resources
Texture NoSignalTexture;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables()
{
    LOG( "Initializing global variables" );
    
    // default bios to load is standard bios
    BiosFileName = "StandardBios.v32";
    
    // we can safely use EmulatorFolder, it will
    // be filled even before this function is
    // called so that logging is initialized
    LastCartridgeDirectory = EmulatorFolder;
    LastMemoryCardDirectory = EmulatorFolder;
}


// =============================================================================
//      CALLBACK FUNCTIONS FOR CONSOLE LOGIC
// =============================================================================


namespace CallbackFunctions
{
    void ClearScreen( V32::GPUColor ClearColor )
    {
        Video.ClearScreen( ClearColor );
    }

    // -----------------------------------------------------------------------------

    void DrawQuad( V32::GPUQuad& DrawnQuad )
    {
        Video.AddQuadToQueue( DrawnQuad );
    }

    // -----------------------------------------------------------------------------

    void SetMultiplyColor( V32::GPUColor NewMultiplyColor )
    {
        // GPU colors are not directly comparable so use words
        V32::V32Word New, Old;
        New.AsColor = NewMultiplyColor;
        Old.AsColor = Video.GetMultiplyColor();
        
        // set multiply color only when needed, so that
        // quad groups are not broken without need
        if( New.AsInteger != Old.AsInteger )
          Video.SetMultiplyColor( NewMultiplyColor );
    }

    // -----------------------------------------------------------------------------

    void SetBlendingMode( int NewBlendingMode )
    {
        // set blending mode only when needed, so that
        // quad groups are not broken without need
        if( NewBlendingMode != (int)Video.GetBlendingMode() )
          Video.SetBlendingMode( (V32::IOPortValues)NewBlendingMode );
    }

    // -----------------------------------------------------------------------------

    void SelectTexture( int GPUTextureID )
    {
        // select texture only when needed, so that
        // quad groups are not broken without need
        if( GPUTextureID != Video.GetSelectedTexture() )
          Video.SelectTexture( GPUTextureID );
    }

    // -----------------------------------------------------------------------------

    void LoadTexture( int GPUTextureID, void* Pixels )
    {
        Video.LoadTexture( GPUTextureID, Pixels );
    }

    // -----------------------------------------------------------------------------

    void UnloadCartridgeTextures()
    {
        for( int i = 0; i < V32::Constants::GPUMaximumCartridgeTextures; i++ )
          Video.UnloadTexture( i );
    }
    
    // -----------------------------------------------------------------------------

    void UnloadBiosTexture()
    {
        Video.UnloadTexture( -1 );
    }
    
    // -----------------------------------------------------------------------------

    void LogLine( const string& Message )
    {
        LOG( Message );
    }
    
    // -----------------------------------------------------------------------------

    void ThrowException( const string& Message )
    {
        THROW( Message );
    }
}
