// *****************************************************************************
    // start include guard
    #ifndef GAMEPADSINPUT_HPP
    #define GAMEPADSINPUT_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
    #include <string>           // [ C++ STL ] Strings
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
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
        
        // optional command button
        SDL_Keycode Command;
};

// -----------------------------------------------------------------------------

// possible types of joystick controls
enum class JoystickControlTypes
{
    None,
    Button,
    Axis,
    Hat
};

// -----------------------------------------------------------------------------

// identification of a single control from a given joystick
class JoystickControl
{
    public:
        
        // control type
        JoystickControlTypes Type;
        
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
        
        // type queries
        bool IsButton() { return (Type == JoystickControlTypes::Button); };
        bool IsAxis()   { return (Type == JoystickControlTypes::Axis  ); };
        bool IsHat()    { return (Type == JoystickControlTypes::Hat   ); };
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
        
        // optional command button
        JoystickControl Command;
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
//      CLASS FOR GAMEPADS INPUT
// =============================================================================


class GamepadsInput
{
    private:
        
        // all currently connected joysticks
        std::map< SDL_JoystickID, SDL_JoystickGUID > ConnectedJoysticks;
        
        // all of our available mappings
        KeyboardMapping KeyboardProfile;
        std::map< SDL_JoystickGUID, JoystickMapping* > JoystickProfiles;
        
        // state of the command button for each gamepad (these are optional
        // and not part of the console gamepads so handle them separately)
        bool CommandPressed[ V32::Constants::GamepadPorts ];
        
    public:
        
        // maps {Vircon gamepads} --> {PC devices}
        DeviceInfo MappedGamepads[ V32::Constants::GamepadPorts ];
        
    private:
        
        // specialized event processing functions
        void ProcessJoystickAdded( SDL_Event Event );
        void ProcessJoystickRemoved( SDL_Event Event );
        void ProcessJoystickAxisMotion( SDL_Event Event );
        void ProcessJoystickHatMotion( SDL_Event Event );
        void ProcessJoystickButtonDown( SDL_Event Event );
        void ProcessJoystickButtonUp( SDL_Event Event );
        void ProcessKeyDown( SDL_Event Event );
        void ProcessKeyUp( SDL_Event Event );
        
    public:
        
        // instance handling
        GamepadsInput();
       ~GamepadsInput();
        
        // handling control profiles
        void SetDefaultProfiles();
        void AddJoystickProfile( SDL_JoystickGUID NewJoystickGUID, JoystickMapping* NewJoystickProfile );
        const std::map< SDL_JoystickGUID, JoystickMapping* >& ReadAllJoystickProfiles();
        JoystickMapping* GetJoystickProfile( const std::string& ProfileName );
        JoystickMapping* GetJoystickProfile( SDL_JoystickGUID GUID );
        KeyboardMapping& GetKeyboardProfile();
        
        // handling devices
        void OpenAllJoysticks();
        void CloseAllJoysticks();
        void AssignInputDevices();
        
        // processing input events
        void ProcessEvent( SDL_Event Event );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
