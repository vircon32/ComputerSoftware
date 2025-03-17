// *****************************************************************************
    // start include guard
    #ifndef SETTINGS_HPP
    #define SETTINGS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <map>              // [ C++ STL ] Maps
    
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
//      GLOBALS FOR INPUT PROFILES
// =============================================================================


// all currently connected joysticks
extern std::map< SDL_JoystickID, SDL_JoystickGUID > ConnectedJoysticks;

// all of our available mappings
extern KeyboardMapping KeyboardProfile;
extern std::map< SDL_JoystickGUID, JoystickMapping* > JoystickProfiles;

// the currently selected joystick profile
// (when NULL, the keyboard mapping is selected)
extern JoystickMapping* SelectedProfile;

// pointers to controls currently being mapped
// (when not NULL, events are used to set their definition)
extern SDL_Keycode* KeyBeingMapped;
extern JoystickControl* ControlBeingMapped;


// =============================================================================
//      LOAD & SAVE INPUT DEVICES FROM XML FILE
// =============================================================================


void SetDefaultControls();
void LoadControls( const std::string& FilePath );
void SaveControls( const std::string& FilePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

