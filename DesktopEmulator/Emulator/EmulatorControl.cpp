// *****************************************************************************
    // include console logic headers
    #include "../ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "Globals.hpp"
    #include "Settings.hpp"
    #include "AudioOutput.hpp"
    #include "VideoOutput.hpp"
    #include "Logger.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <climits>          // [ ANSI C ] Numeric limits
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      CLASS: EMULATOR CONTROL
// =============================================================================


EmulatorControl::EmulatorControl()
{
    Paused = false;
}

// -----------------------------------------------------------------------------

EmulatorControl::~EmulatorControl()
{
    Terminate();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Initialize()
{
    Audio.Initialize();
    
    Console.SetCallbackClearScreen( Function_ClearScreen );
    Console.SetCallbackDrawQuad( Function_DrawQuad );
    Console.SetCallbackSetMultiplyColor( Function_SetMultiplyColor );
    Console.SetCallbackSetBlendingMode( Function_SetBlendingMode );
    Console.SetCallbackSelectTexture( Function_SelectTexture );
    Console.SetCallbackLoadTexture( Function_LoadTexture );
    Console.SetCallbackUnloadCartridgeTextures( Function_UnloadCartridgeTextures );
}

// -----------------------------------------------------------------------------

void EmulatorControl::Terminate()
{
    Console.SetPower( false );
    Audio.Terminate();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Pause()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || Paused ) return;
    
    // take pause actions
    Paused = true;
    Audio.Pause();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Resume()
{
    // do nothing when not applicable
    if( !Console.IsPowerOn() || !Paused ) return;
    
    // take resume actions
    Paused = false;
    Audio.Resume();
}

// -----------------------------------------------------------------------------

bool EmulatorControl::IsPaused()
{
    return Paused;
}

// -----------------------------------------------------------------------------

void EmulatorControl::SetPower( bool On )
{
    Video.RenderToFramebuffer();
    Console.SetPower( On );

    if( On ) Audio.Reset();
    else Audio.Pause();
}

// -----------------------------------------------------------------------------

bool EmulatorControl::IsPowerOn()
{
    return Console.IsPowerOn();
}

// -----------------------------------------------------------------------------

void EmulatorControl::Reset()
{
    LOG( "EmulatorControl::Reset" );
    Paused = false;
    Video.RenderToFramebuffer();
    Console.Reset();
    Audio.Reset();
}

// -----------------------------------------------------------------------------

void EmulatorControl::RunNextFrame()
{
    Console.RunNextFrame();
    Audio.ChangeFrame();
    
    // after running, ensure that all GPU
    // commands run in the current frame are drawn
    glFlush();   
}


// =============================================================================
//      EMULATOR CONTROL: EVENT PROCESSING
// =============================================================================


void EmulatorControl::ProcessEvent( SDL_Event Event )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // JOYSTICK CONNECTION EVENTS
    
    if( Event.type == SDL_JOYDEVICEADDED )
    {
        // access the joystick
        SDL_Joystick* NewJoystick = SDL_JoystickOpen( Event.jdevice.which );
        
        if( NewJoystick )
        {
            // find out joystick instance ID and GUID
            SDL_JoystickGUID NewGUID = SDL_JoystickGetGUID( NewJoystick );
            Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
            
            // update the list of connected joysticks
            ConnectedJoysticks[ AddedInstanceID ] = NewGUID;
        }
        
        // detect joysticks and assign them to gamepads
        AssignInputDevices();
    }
    
    else if( Event.type == SDL_JOYDEVICEREMOVED )
    {
        // find out joystick instance ID and GUID
        Sint32 RemovedInstanceID = Event.jdevice.which;
        SDL_Joystick* OldJoystick = SDL_JoystickFromInstanceID( RemovedInstanceID );
        
        // update the list of connected joysticks
        ConnectedJoysticks.erase( RemovedInstanceID );
        SDL_JoystickClose( OldJoystick );
        
        // detect joysticks and assign them to gamepads
        AssignInputDevices();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // KEYBOARD INPUT EVENTS
    
    else if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
    {
        SDL_Keycode KeyCode = Event.key.keysym.sym;
        
        // ignore keypresses when control is pressed,
        // so that keyboard shortcuts will not interfere
        bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
        if( ControlIsPressed ) return;
        
        // in other cases process the key normally
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is the keyboard
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
              continue;
            
            // check the mapped keys for directions
            if( KeyCode == KeyboardProfile.Left )
              Console.SetGamepadControl( Gamepad, GamepadControls::Left, true );
              
            if( KeyCode == KeyboardProfile.Right )
              Console.SetGamepadControl( Gamepad, GamepadControls::Right, true );
              
            if( KeyCode == KeyboardProfile.Up )
              Console.SetGamepadControl( Gamepad, GamepadControls::Up, true );
              
            if( KeyCode == KeyboardProfile.Down )
              Console.SetGamepadControl( Gamepad, GamepadControls::Down, true );
              
            // check the mapped keys for buttons
            if( KeyCode == KeyboardProfile.ButtonA )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, true );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, true );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, true );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, true );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, true );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, true );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, true );
        }
    }
    
    else if( Event.type == SDL_KEYUP )
    {
        SDL_Keycode KeyCode = Event.key.keysym.sym;
        
        // ignore keypresses when control is pressed,
        // so that keyboard shortcuts will not interfere
        bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
        if( ControlIsPressed ) return;
        
        // in other cases process the key normally
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is the keyboard
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
              continue;
            
            // check the mapped keys for directions
            if( KeyCode == KeyboardProfile.Left )
              Console.SetGamepadControl( Gamepad, GamepadControls::Left, false );
              
            if( KeyCode == KeyboardProfile.Right )
              Console.SetGamepadControl( Gamepad, GamepadControls::Right, false );
              
            if( KeyCode == KeyboardProfile.Up )
              Console.SetGamepadControl( Gamepad, GamepadControls::Up, false );
              
            if( KeyCode == KeyboardProfile.Down )
              Console.SetGamepadControl( Gamepad, GamepadControls::Down, false );
              
            // check the mapped keys for buttons
            if( KeyCode == KeyboardProfile.ButtonA )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, false );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, false );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, false );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, false );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, false );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, false );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, false );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // JOYSTICK INPUT EVENTS
    
    else if( Event.type == SDL_JOYAXISMOTION )
    {
        Uint8 AxisIndex = Event.jaxis.axis;
        Sint16 AxisPosition = Event.jaxis.value;
        Sint32 InstanceID = Event.jaxis.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        // we need to process both directions in this axis
        // at the same time, because they are correlated.
        // But be careful because it could happen that not
        // both directions have been mapped
        
        // joystick could be analog, so allow for
        // a dead zone in the center of +/- 50%
        bool PositivePressed = (AxisPosition > +16000);
        bool NegativePressed = (AxisPosition < -16000);
        
        // check all gamepads
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
              
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped axes for directions
            if( JoystickProfile->Left.IsAxis )
              if( AxisIndex == JoystickProfile->Left.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Left, JoystickProfile->Left.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Right.IsAxis )
              if( AxisIndex == JoystickProfile->Right.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Right, JoystickProfile->Right.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Up.IsAxis )
              if( AxisIndex == JoystickProfile->Up.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Up, JoystickProfile->Up.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->Down.IsAxis )
              if( AxisIndex == JoystickProfile->Down.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Down, JoystickProfile->Down.AxisPositive? PositivePressed : NegativePressed );
            
            // check the mapped axes for buttons
            if( JoystickProfile->ButtonA.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonA.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, JoystickProfile->ButtonA.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonB.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonB.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, JoystickProfile->ButtonB.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonX.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonX.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, JoystickProfile->ButtonX.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonY.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonY.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, JoystickProfile->ButtonY.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonL.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonL.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, JoystickProfile->ButtonL.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonR.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonR.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, JoystickProfile->ButtonR.AxisPositive? PositivePressed : NegativePressed );
            
            if( JoystickProfile->ButtonStart.IsAxis )
              if( AxisIndex == JoystickProfile->ButtonStart.AxisIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, JoystickProfile->ButtonStart.AxisPositive? PositivePressed : NegativePressed );
        }
    }
    
    else if( Event.type == SDL_JOYBUTTONDOWN )
    {
        Uint8 ButtonIndex = Event.jbutton.button;
        Sint32 InstanceID = Event.jbutton.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped buttons for directions
            if( !JoystickProfile->Left.IsAxis )
              if( ButtonIndex == JoystickProfile->Left.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Left, true );
              
            if( !JoystickProfile->Right.IsAxis )
              if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Right, true );
              
            if( !JoystickProfile->Up.IsAxis )
              if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Up, true );
              
            if( !JoystickProfile->Down.IsAxis )
              if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Down, true );
              
            // check the mapped buttons for buttons
            if( !JoystickProfile->ButtonA.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, true );
            
            if( !JoystickProfile->ButtonB.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, true );
            
            if( !JoystickProfile->ButtonX.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, true );
            
            if( !JoystickProfile->ButtonY.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, true );
              
            if( !JoystickProfile->ButtonL.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, true );
            
            if( !JoystickProfile->ButtonR.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, true );
            
            if( !JoystickProfile->ButtonStart.IsAxis )
              if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, true );
        }
    }
    
    else if( Event.type == SDL_JOYBUTTONUP )
    {
        Uint8 ButtonIndex = Event.jbutton.button;
        Sint32 InstanceID = Event.jbutton.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped buttons for directions
            if( ButtonIndex == JoystickProfile->Left.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::Left, false );
              
            if( ButtonIndex == JoystickProfile->Right.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::Right, false );
              
            if( ButtonIndex == JoystickProfile->Up.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::Up, false );
              
            if( ButtonIndex == JoystickProfile->Down.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::Down, false );
              
            // check the mapped buttons for buttons
            if( ButtonIndex == JoystickProfile->ButtonA.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, false );
            
            if( ButtonIndex == JoystickProfile->ButtonB.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, false );
            
            if( ButtonIndex == JoystickProfile->ButtonX.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, false );
            
            if( ButtonIndex == JoystickProfile->ButtonY.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, false );
              
            if( ButtonIndex == JoystickProfile->ButtonL.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, false );
            
            if( ButtonIndex == JoystickProfile->ButtonR.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, false );
            
            if( ButtonIndex == JoystickProfile->ButtonStart.ButtonIndex )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, false );
        }
    }
    
    else if( Event.type == SDL_JOYHATMOTION )
    {
        Uint8 HatIndex = Event.jhat.hat;
        Uint8 HatDirection = Event.jhat.value;
        Sint32 InstanceID = Event.jhat.which;
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
        SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
        
        // we need to process both axes together, and
        // for each axis we need to process both directions
        
        // check all gamepads
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // non-connected gamepads are ignored
            if( !Console.HasGamepad( Gamepad ) )
              continue;
            
            // check if mapped device is a joystick
            if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
              continue;
            
            // check if mapped device is this specific joystick
            if( MappedGamepads[ Gamepad ].InstanceID != InstanceID
            ||  MappedGamepads[ Gamepad ].GUID != GUID )
              continue;
            
            // obtain the applicable joystick profile
            auto Position = JoystickProfiles.find( GUID );
            
            if( Position == JoystickProfiles.end() )
              continue;
            
            JoystickMapping* JoystickProfile = Position->second;
            
            // check the mapped axes for directions
            if( JoystickProfile->Left.IsHat )
              if( HatIndex == JoystickProfile->Left.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Left, (bool)(HatDirection & JoystickProfile->Left.HatDirection) );
            
            if( JoystickProfile->Right.IsHat )
              if( HatIndex == JoystickProfile->Right.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Right, (bool)(HatDirection & JoystickProfile->Right.HatDirection) );
            
            if( JoystickProfile->Up.IsHat )
              if( HatIndex == JoystickProfile->Up.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Up, (bool)(HatDirection & JoystickProfile->Up.HatDirection) );
            
            if( JoystickProfile->Down.IsHat )
              if( HatIndex == JoystickProfile->Down.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::Down, (bool)(HatDirection & JoystickProfile->Down.HatDirection) );
            
            // check the mapped buttons for buttons
            if( !JoystickProfile->ButtonA.IsHat )
              if( HatIndex == JoystickProfile->ButtonA.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, (bool)(HatDirection & JoystickProfile->ButtonA.HatDirection) );
            
            if( !JoystickProfile->ButtonB.IsHat )
              if( HatIndex == JoystickProfile->ButtonB.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, (bool)(HatDirection & JoystickProfile->ButtonB.HatDirection) );
            
            if( !JoystickProfile->ButtonX.IsHat )
              if( HatIndex == JoystickProfile->ButtonX.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, (bool)(HatDirection & JoystickProfile->ButtonX.HatDirection) );
            
            if( !JoystickProfile->ButtonY.IsHat )
              if( HatIndex == JoystickProfile->ButtonY.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, (bool)(HatDirection & JoystickProfile->ButtonY.HatDirection) );
            
            if( !JoystickProfile->ButtonL.IsHat )
              if( HatIndex == JoystickProfile->ButtonL.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, (bool)(HatDirection & JoystickProfile->ButtonL.HatDirection) );
            
            if( !JoystickProfile->ButtonR.IsHat )
              if( HatIndex == JoystickProfile->ButtonR.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, (bool)(HatDirection & JoystickProfile->ButtonR.HatDirection) );
            
            if( !JoystickProfile->ButtonStart.IsHat )
              if( HatIndex == JoystickProfile->ButtonStart.HatIndex )
                Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, (bool)(HatDirection & JoystickProfile->ButtonStart.HatDirection) );
        }
    }
}
