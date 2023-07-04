// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "Globals.hpp"
    #include "V32Emulator.hpp"
    
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
//      VIDEO OBJECTS
// =============================================================================


OpenGL2DContext OpenGL2D;

string VertexShader =
    "#version 100" "\n"
    "" "\n"
    "attribute vec2 position;" "\n"
    "attribute vec2 inputTextureCoordinate;" "\n"
    "varying highp vec2 textureCoordinate;" "\n"
    "" "\n"
    "void main()" "\n"
    "{" "\n"
    "    // x is transformed from (0.0,639.0) to (-1.0,+1.0)" "\n"
    "    // y is transformed from (0.0,359.0) to (+1.0,-1.0), so it becomes inverted" "\n"
    "    gl_Position = vec4( (position.x / (639.0/2.0)) - 1.0, 1.0 - (position.y / (359.0/2.0)), 0.0, 1.0 );" "\n"
    "    textureCoordinate = inputTextureCoordinate;" "\n"
    "}";

string FragmentShader =
    "#version 100" "\n"
    "" "\n"
    "uniform sampler2D textureUnit;" "\n"
    "uniform mediump vec4 multiplyColor;" "\n"
    "varying highp vec2 textureCoordinate;" "\n"
    "" "\n"
    "void main()" "\n"
    "{" "\n"
    "    gl_FragColor = multiplyColor * texture2D(textureUnit, textureCoordinate);" "\n"
    "}";


// =============================================================================
//      PROGRAM OBJECTS
// =============================================================================


// video objects
Texture NoSignalTexture;

// instance of the Vircon virtual machine
V32Emulator Vircon;


// =============================================================================
//      INITIALIZATION OF VARIABLES
// =============================================================================


void InitializeGlobalVariables()
{
    LOG( "Initializing global variables" );
    
    // default bios is standard bios
    BiosFileName = "StandardBios.v32";
    
    // we can safely use EmulatorFolder, it will
    // be filled even before this function is
    // called so that logging is initialized
    LastCartridgeDirectory = EmulatorFolder;
    LastMemoryCardDirectory = EmulatorFolder;
}
