// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/Logger.hpp"
    
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "GamepadsInput.hpp"
    #include "VideoOutput.hpp"
    #include "AudioOutput.hpp"
    #include "Texture.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
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
V32Console Console;

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
        Video.DrawTexturedQuad( DrawnQuad );
    }

    // -----------------------------------------------------------------------------

    void SetMultiplyColor( V32::GPUColor MultiplyColor )
    {
        Video.SetMultiplyColor( MultiplyColor );
    }

    // -----------------------------------------------------------------------------

    void SetBlendingMode( int NewBlendingMode )
    {
        Video.SetBlendingMode( (IOPortValues)NewBlendingMode );
    }

    // -----------------------------------------------------------------------------

    void SelectTexture( int GPUTextureID )
    {
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
        for( int i = 0; i < Constants::GPUMaximumCartridgeTextures; i++ )
          Video.UnloadTexture( i );
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
