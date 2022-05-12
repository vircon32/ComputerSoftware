// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "VirconGamepadController.hpp"
    
    // include C/C++ headers
    #include <string.h>         // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: VIRCON GAMEPAD CONTROLLER
// =============================================================================


VirconGamepadController::VirconGamepadController()
{
    // for all gamepad ports
    for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
    {
        // reset the gamepad
        ResetGamepad( Gamepad );
        
        // start with no connected gamepads
        RealTimeGamepadStates[ Gamepad ].Connected = false;
    }
    
    // set a known initial state
    Reset();
}

// -----------------------------------------------------------------------------

bool VirconGamepadController::ReadPort( int32_t LocalPort, VirconWord& Result )
{
    // check range
    if( LocalPort > INP_LastPort )
      return false;
    
    // global ports
    if( LocalPort == (int32_t)INP_LocalPorts::SelectedGamepad )
      Result.AsInteger = SelectedGamepad;
    
    // gamepad-specific ports
    VirconWord* PortArray = (VirconWord*)(&ProvidedGamepadStates[ SelectedGamepad ]);
    Result = PortArray[ LocalPort - 1 ];
    return true;
}

// -----------------------------------------------------------------------------

bool VirconGamepadController::WritePort( int32_t LocalPort, VirconWord Value )
{
    // only the active gamepad register can be written to
    if( LocalPort != (int32_t)INP_LocalPorts::SelectedGamepad )
      return false;
    
    // write the value only if the range is correct
    if( Value.AsInteger >= 0 && Value.AsInteger < Constants::MaximumGamepads )
      SelectedGamepad = Value.AsInteger;
    
    return true;
}

// -----------------------------------------------------------------------------

void VirconGamepadController::ChangeFrame()
{
    // first provide current states
    for( int i = 0; i < Constants::MaximumGamepads; i++ )
      ProvidedGamepadStates[ i ] = RealTimeGamepadStates[ i ];
    
    // now increase all counts by 1 for next frame
    // (not including the connection indicator, which is a boolean)
    for( int Gamepad = 0; Gamepad < Constants::MaximumGamepads; Gamepad++ )
    {
        int32_t* TimeCount = (int32_t*)(&RealTimeGamepadStates[ Gamepad ]);
        
        for( int i = 1; i <= 11; i++ )
        {
            if( TimeCount[i] < 0 ) TimeCount[i]--;
            else                   TimeCount[i]++;
            
            // keep values within a 1-minute range
            Clamp( TimeCount[i], -3600, 3600 );
        }
    }
}

// -----------------------------------------------------------------------------

void VirconGamepadController::Reset()
{
    // set the first gamepad as selected
    SelectedGamepad = 0;
    
    // do NOT alter the state of gamepads! (their connection
    // and presses are independent of console power/resets)
}

// -----------------------------------------------------------------------------

void VirconGamepadController::ResetGamepad( int GamepadPort )
{
    // reject invalid requests
    if( GamepadPort >= Constants::MaximumGamepads )
      return;
    
    // all times states are set to 1 minute unpressed
    int32_t* GamepadPresses = &RealTimeGamepadStates[ GamepadPort ].Left;
    
    for( int i = 0; i < 11; i++ )
    {
        *GamepadPresses = -3600;
        GamepadPresses++;
    }
    
    // copy that to the provided states
    memcpy( &ProvidedGamepadStates[ GamepadPort ], &RealTimeGamepadStates[ GamepadPort ], sizeof(GamepadState) );
}

// -----------------------------------------------------------------------------

void VirconGamepadController::ProcessConnectionChange( int GamepadPort, bool Connected )
{
    // reject invalid events
    if( GamepadPort >= Constants::MaximumGamepads )
      return;
    
    // change value
    RealTimeGamepadStates[ GamepadPort ].Connected = Connected;
    
    // on disconnection events, reset the state of all buttons and directions
    if( !Connected )
      ResetGamepad( GamepadPort );
}

// -----------------------------------------------------------------------------

void VirconGamepadController::ProcessButtonChange( int GamepadPort, GamepadButtons Button, bool Pressed )
{
    // reject invalid events
    if( GamepadPort >= Constants::MaximumGamepads )
      return;
    
    // ignore buttons for non connected gamepads
    if( !RealTimeGamepadStates[ GamepadPort ].Connected )
      return;
    
    // do not process redundant events
    // (otherwise times would incorrectly reset)
    int32_t* ButtonStates = &RealTimeGamepadStates[ GamepadPort ].ButtonStart;
    bool WasPressed = (ButtonStates[ (int)Button ] > 0);
    
    if( Pressed == WasPressed )
      return;
    
    // change value
    ButtonStates[ (int)Button ] = (Pressed? 1 : -1);
}

// -----------------------------------------------------------------------------

void VirconGamepadController::ProcessDirectionChange( int GamepadPort, GamepadDirections Direction, bool Pressed )
{
    // reject invalid events
    if( GamepadPort >= Constants::MaximumGamepads )
      return;
    
    // ignore directions for non connected gamepads
    if( !RealTimeGamepadStates[ GamepadPort ].Connected )
      return;
    
    // do not process redundant events
    // (otherwise times would incorrectly reset)
    int32_t* DirectionStates = &RealTimeGamepadStates[ GamepadPort ].Left;
    bool WasPressed = (DirectionStates[ (int)Direction ] > 0);
    
    if( Pressed == WasPressed )
      return;
    
    // change value
    DirectionStates[ (int)Direction ] = (Pressed? 1 : -1);
    
    // when a new direction becomes pressed, ensure that
    // opposite directions can never be pressed simultaneously
    // (but again, avoid reseting time on redundancies)
    if( !Pressed ) return;
    
    switch( Direction )
    {
        case GamepadDirections::Left:
            if( RealTimeGamepadStates[ GamepadPort ].Right > 0 )
              RealTimeGamepadStates[ GamepadPort ].Right = -1;
            break;
        case GamepadDirections::Right:
            if( RealTimeGamepadStates[ GamepadPort ].Left > 0 )
              RealTimeGamepadStates[ GamepadPort ].Left = -1;
            break;
        case GamepadDirections::Up:
            if( RealTimeGamepadStates[ GamepadPort ].Down > 0 )
              RealTimeGamepadStates[ GamepadPort ].Down = -1;
            break;
        case GamepadDirections::Down:
            if( RealTimeGamepadStates[ GamepadPort ].Up > 0 )
              RealTimeGamepadStates[ GamepadPort ].Up = -1;
            break;
        default: break;
    }
}

// -----------------------------------------------------------------------------

bool VirconGamepadController::IsGamepadConnected( int GamepadPort )
{
    if( GamepadPort < 0 || GamepadPort >= Constants::MaximumGamepads )
      return false;
    
    return RealTimeGamepadStates[ GamepadPort ].Connected;
}
