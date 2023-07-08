// *****************************************************************************
    // start include guard
    #ifndef V32GAMEPADCONTROLLER_HPP
    #define V32GAMEPADCONTROLLER_HPP
    
    // include project headers
    #include "ExternalInterfaces.hpp"
    #include "V32Buses.hpp"
// *****************************************************************************


namespace V32
{
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
    
    
    // =============================================================================
    //      INPUT CONTROLLER
    // =============================================================================
    
    
    class V32GamepadController: public VirconControlInterface
    {
        public:
            
            // currently selected gamepad
            int32_t SelectedGamepad;
            
            // state of the 4 gamepads
            GamepadState RealTimeGamepadStates[ Constants::GamepadPorts ];
            GamepadState ProvidedGamepadStates[ Constants::GamepadPorts ];
            
            // mapping configuration
            // (pending)
            
        public:
            
            // instance handling
            V32GamepadController();
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
            
            // general operation
            void ChangeFrame();
            void Reset();
            void ResetGamepad( int GamepadPort );
            
            // gamepad events
            void SetGamepadConnection( int GamepadPort, bool Connected );
            void SetGamepadControl( int GamepadPort, GamepadControls Control, bool Pressed );
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

