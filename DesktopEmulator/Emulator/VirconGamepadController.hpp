// *****************************************************************************
    // start include guard
    #ifndef VIRCONGAMEPADCONTROLLER_HPP
    #define VIRCONGAMEPADCONTROLLER_HPP
    
    // include infrastructure headers
    #include "../DesktopInfrastructure/Definitions.hpp"
    
    // include project headers
    #include "VirconBuses.hpp"
// *****************************************************************************


// =============================================================================
//      INPUT DEFINITIONS
// =============================================================================


// LOCAL port numbers!
// (to be added the initial address)
enum class INP_LocalPorts: int32_t
{
    SelectedGamepad = 0,
    
    GamepadConnected,
    GamepadLeft,
    GamepadRight,
    GamepadUp,
    GamepadDown,
    GamepadButtonStart,
    GamepadButtonA,
    GamepadButtonB,
    GamepadButtonX,
    GamepadButtonY,
    GamepadButtonL,
    GamepadButtonR
};

// used as limit of local port numbers
const int32_t INP_LastPort = (int32_t)INP_LocalPorts::GamepadButtonR;

// -----------------------------------------------------------------------------

typedef struct
{
    int32_t Connected;
    int32_t Left;
    int32_t Right;
    int32_t Up;
    int32_t Down;
    int32_t ButtonStart;
    int32_t ButtonA;
    int32_t ButtonB;
    int32_t ButtonX;
    int32_t ButtonY;
    int32_t ButtonL;
    int32_t ButtonR;
}
GamepadState;

// -----------------------------------------------------------------------------

// button codes for input events
enum class GamepadButtons
{
    Start = 0,
    A, B, X, Y, L, R
};

// -----------------------------------------------------------------------------

// directions codes for input events
enum class GamepadDirections
{
    Left = 0,
    Right,
    Up,
    Down
};


// =============================================================================
//      INPUT CONTROLLER
// =============================================================================


class VirconGamepadController: public VirconControlInterface
{
    public:
        
        // currently selected gamepad
        int32_t SelectedGamepad;
        
        // state of the 4 gamepads
        GamepadState RealTimeGamepadStates[ Constants::MaximumGamepads ];
        GamepadState ProvidedGamepadStates[ Constants::MaximumGamepads ];
        
        // mapping configuration
        // (pending)
        
    public:
        
        // instance handling
        VirconGamepadController();
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
        // general operation
        void ChangeFrame();
        void Reset();
        void ResetGamepad( int GamepadPort );
        
        // gamepad events
        void ProcessConnectionChange( int GamepadPort, bool Connected );
        void ProcessButtonChange( int GamepadPort, GamepadButtons Button, bool Pressed );
        void ProcessDirectionChange( int GamepadPort, GamepadDirections Direction, bool Pressed );
        
        // external queries
        bool IsGamepadConnected( int GamepadPort );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

