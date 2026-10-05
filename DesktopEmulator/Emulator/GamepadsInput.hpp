// *****************************************************************************
    // start include guard
    #ifndef GAMEPADSINPUT_HPP
    #define GAMEPADSINPUT_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
    #include <deque>            // [ C++ STL ] Double-ended queues
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
    Keyboard,       // a few keys mapped to gamepad controls
    Joystick,
    V32Kbd          // full keyboard: scancodes encoded as gamepad controls
};

// -----------------------------------------------------------------------------

// name used for the v32kbd device, both in the
// gamepads menu and as profile name in settings
#define V32KBD_PROFILE_NAME "v32kbd"

// A v32kbd device presents itself to the console as a regular gamepad,
// but its 11 controls are used to report keyboard events:
//
//   Start, A, B, X, Y, L, R --> 7-bit key code (Start = bit 0 ... R = bit 6),
//                               in the same order as their IO ports
//   Up / Down               --> key action: Up = pressed, Down = released
//   Left / Right            --> strobe: every new key event alternates
//                               between Left and Right, beginning by Left
//
// The d-pad never has opposite directions pressed together, so this is
// always a valid gamepad state and no console logic needs to be changed.
//
// Key codes identify keys (not characters: no shift is applied). Keys
// with an ASCII character use it as their code, taking that key in a US
// layout with no shift: 'a'-'z', '0'-'9', space and  ` - = [ ] \ ; ' , . /
// The other keys use the codes listed in the enumeration below.
//
// Only 1 key event is reported per frame, and controls keep their last
// reported state until the next event. A new event can be detected when
// the pressed side (Left/Right) changes. Before the first event, all
// controls are unpressed.
namespace V32Kbd
{
    enum KeyCodes
    {
        Key_None = 0,       // never reported
        Key_Up = 1,
        Key_Down,
        Key_Left,
        Key_Right,
        Key_CapsLock = 5,
        Key_LeftShift,
        Key_RightShift,
        Key_Backspace = 8,  // same as ASCII
        Key_Tab = 9,        // same as ASCII
        Key_LeftControl = 10,
        Key_RightControl,
        Key_LeftAlt = 12,   // Option key on Mac
        Key_Enter = 13,     // same as ASCII
        Key_F1 = 14,        // F1 to F12 are consecutive: 14 to 25
        Key_F12 = 25,
        Key_RightAlt = 26,  // Option key on Mac
        Key_Escape = 27,    // same as ASCII
        Key_LeftGUI = 28,   // Command key on Mac, Windows key on PC
        Key_RightGUI = 29,
                            // 30 and 31 are unused
        Key_Delete = 127    // same as ASCII
    };
    
    // number of bits in a key code
    const int CodeBits = 7;
    
    // pending key events over this limit are discarded
    const unsigned MaxQueuedEvents = 256;
    
    // gamepad control used to report each bit of the key code
    extern const V32::GamepadControls CodeControls[ CodeBits ];
    
    // converts host keys to key codes (Key_None for unsupported keys)
    int GetKeyCode( SDL_Scancode Scancode );
}

// a single key event waiting to be reported
struct V32KbdEvent
{
    uint8_t KeyCode;
    bool Pressed;
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
        
        // key events pending to be reported by the v32kbd device
        std::deque< V32KbdEvent > V32KbdQueue;
        
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
        void ProcessV32KbdKey( SDL_Event Event );
        
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
        
        // queries on device usage (-1 = not used in any gamepad)
        int GetKeyboardGamepad();
        int GetV32KbdGamepad();
        
        // v32kbd device: call this exactly once before every emulated
        // frame, to report the next pending key event (if there is any)
        void UpdateV32Kbd();
        
        // processing input events
        void ProcessEvent( SDL_Event Event );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
