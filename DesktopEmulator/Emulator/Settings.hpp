// *****************************************************************************
    // start include guard
    #ifndef SETTINGS_HPP
    #define SETTINGS_HPP
    
    // include project headers
    #include "V32GamepadController.hpp"
    
    // include C/C++ headers
    #include <string>               // [ C++ STL ] Strings
    #include <map>                  // [ C++ STL ] Maps
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR INPUT MAPPINGS
// =============================================================================


// control mapping for the keyboard
class KeyboardMapping
{
    public:
        
        // d-pad directions
        SDL_Keycode Left, Right, Up, Down;
        
        // buttons
        SDL_Keycode ButtonA, ButtonB, ButtonX, ButtonY;
        SDL_Keycode ButtonL, ButtonR, ButtonStart;
};

// -----------------------------------------------------------------------------

// identification of a single control from a given joystick
class JoystickControl
{
    public:
        
        // control type
        bool IsAxis;
        bool IsHat;
        
        // button info
        int ButtonIndex;
        
        // axis info
        int AxisIndex;
        bool AxisPositive;
        
        // hat info
        int HatIndex;
        int HatDirection;
    
    public:
        
        // constructor to leave all controls unmapped
        JoystickControl();
};

// -----------------------------------------------------------------------------

// control mapping for a joystick
class JoystickMapping
{
    public:
        
        // static identification
        SDL_JoystickGUID GUID;
        
        // human-readable names
        std::string ProfileName;
        std::string JoystickName;
        
        // d-pad directions
        JoystickControl Left, Right, Up, Down;
        
        // buttons
        JoystickControl ButtonA, ButtonB, ButtonX, ButtonY;
        JoystickControl ButtonL, ButtonR, ButtonStart;    
};

// -----------------------------------------------------------------------------

// possible options for a mapped host device
enum class DeviceTypes
{
    NoDevice,
    Keyboard,
    Joystick
};

// -----------------------------------------------------------------------------

// full identification of a host computer device
struct DeviceInfo
{
    // base device info
    DeviceTypes Type;
    
    // for a joystick, extra info is needed
    // since there can be several connected
    SDL_JoystickGUID GUID;        // joystick static identification
    SDL_JoystickID InstanceID;    // joystick dynamic identification
};


// =============================================================================
//      OPERATION WITH GUIDS
// =============================================================================


// operators needed to use GUIDs in a std::map
bool operator==( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 );
bool operator!=( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 );
bool operator<( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 );

// GUID <-> string conversions
std::string GUIDToString( SDL_JoystickGUID GUID );
bool GUIDStringIsValid( const std::string& GUIDString );


// =============================================================================
//      GLOBALS FOR INPUT DEVICES
// =============================================================================


// all currently connected joysticks
extern std::map< SDL_JoystickID, SDL_JoystickGUID > ConnectedJoysticks;

// all of our available mappings
extern KeyboardMapping KeyboardProfile;
extern std::map< SDL_JoystickGUID, JoystickMapping* > JoystickProfiles;

// maps {Vircon gamepads} --> {PC devices}
extern DeviceInfo MappedGamepads[ Constants::MaximumGamepads ];


// =============================================================================
//      LOAD & SAVE INPUT DEVICES FROM XML FILE
// =============================================================================


void SetDefaultControls();
void LoadControls( const std::string& FilePath );
void AssignInputDevices();


// =============================================================================
//      LOAD & SAVE SETTINGS FROM XML FILES
// =============================================================================


void SetDefaultSettings();
void LoadSettings( const std::string& FilePath );
void SaveSettings( const std::string& FilePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

