// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/FilePaths.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include controls editor headers
    #include "OpenGL2DContext.hpp"
    #include "LoadTexture.hpp"
    #include "Controls.hpp"
    #include "Globals.hpp"
    #include "GUI.hpp"
    #include "Languages.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <list>         // [ C++ STL ] Lists
    #include <map>          // [ C++ STL ] Maps
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
    #include <SDL2/SDL_opengl.h>    // [ SDL2 ] OpenGL interface
    #include <SDL2/SDL_joystick.h>  // [ SDL2 ] Joystick functions
    #include <SDL2/SDL_image.h>     // [ SDL2 ] SDL_Image
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PROCESSING INPUT EVENTS
// =============================================================================


void ProcessKeyboardKeyDown( SDL_KeyboardEvent KeyEvent )
{
    // discard autorepeat presses
    if( KeyEvent.repeat )
      return;
    
    // find the pressed key
    SDL_Keycode Key = KeyEvent.keysym.sym;
    
    // save the pressed key
    // (but, escape key cancels)
    if( KeyBeingMapped )
    {
        if( Key != SDLK_ESCAPE )
          *KeyBeingMapped = Key;
          
        KeyBeingMapped = nullptr;
    }
    
    // escape key can also cancel
    // the wait for joystick events
    if( ControlBeingMapped )
      if( Key == SDLK_ESCAPE )
        ControlBeingMapped = nullptr;
}

// -----------------------------------------------------------------------------

void ProcessJoystickButtonDown( SDL_JoyButtonEvent ButtonEvent )
{
    // we only press events on wait mode
    if( !ControlBeingMapped )
      return;
      
    // discard events from other joysticks
    Sint32 InstanceID = ButtonEvent.which;
    SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
    SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
    
    if( !SelectedProfile || SelectedProfile->GUID != GUID )
      return;
    
    // find the pressed button
    Uint8 ButtonIndex = ButtonEvent.button;
    
    // save the pressed button
    ControlBeingMapped->IsAxis = false;
    ControlBeingMapped->ButtonIndex = ButtonIndex;
    
    // exit wait mode
    ControlBeingMapped = nullptr;
}

// -----------------------------------------------------------------------------

void ProcessJoystickAxisMotion( SDL_JoyAxisEvent AxisEvent )
{
    // we only press events on wait mode
    if( !ControlBeingMapped )
      return;
    
    // discard events from other joysticks
    Sint32 InstanceID = AxisEvent.which;
    SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
    SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
    
    if( !SelectedProfile || SelectedProfile->GUID != GUID )
      return;
    
    // find the axis position
    Uint8 AxisIndex = AxisEvent.axis;
    Sint16 AxisPosition = AxisEvent.value;
    
    // joystick could be analog, so allow for
    // a dead zone in the center of +/- 50%
    bool PositivePressed = (AxisPosition > +16000);
    bool NegativePressed = (AxisPosition < -16000);
    
    if( PositivePressed )
    {
        // save the moved axis
        ControlBeingMapped->IsAxis = true;
        ControlBeingMapped->AxisPositive = true;
        ControlBeingMapped->AxisIndex = AxisIndex;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
    
    else if( NegativePressed )
    {
        // save the moved axis
        ControlBeingMapped->IsAxis = true;
        ControlBeingMapped->AxisPositive = false;
        ControlBeingMapped->AxisIndex = AxisIndex;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
}

// -----------------------------------------------------------------------------

void ProcessJoystickHatMotion( SDL_JoyHatEvent HatEvent )
{
    // we only press events on wait mode
    if( !ControlBeingMapped )
      return;
    
    // discard events from other joysticks
    Sint32 InstanceID = HatEvent.which;
    SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( InstanceID );
    SDL_JoystickGUID GUID = SDL_JoystickGetGUID( Joystick );
    
    if( !SelectedProfile || SelectedProfile->GUID != GUID )
      return;
    
    // find the hat position
    Uint8 HatIndex = HatEvent.hat;
    Uint8 HatPosition = HatEvent.value;
    
    if( HatPosition & SDL_HAT_LEFT )
    {
        // save the moved hat
        ControlBeingMapped->IsHat = true;
        ControlBeingMapped->HatIndex = HatIndex;
        ControlBeingMapped->HatDirection = SDL_HAT_LEFT;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
    
    else if( HatPosition & SDL_HAT_RIGHT )
    {
        // save the moved hat
        ControlBeingMapped->IsHat = true;
        ControlBeingMapped->HatIndex = HatIndex;
        ControlBeingMapped->HatDirection = SDL_HAT_RIGHT;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
    
    else if( HatPosition & SDL_HAT_UP )
    {
        // save the moved hat
        ControlBeingMapped->IsHat = true;
        ControlBeingMapped->HatIndex = HatIndex;
        ControlBeingMapped->HatDirection = SDL_HAT_UP;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
    
    else if( HatPosition & SDL_HAT_DOWN )
    {
        // save the moved hat
        ControlBeingMapped->IsHat = true;
        ControlBeingMapped->HatIndex = HatIndex;
        ControlBeingMapped->HatDirection = SDL_HAT_DOWN;
        
        // exit wait mode
        ControlBeingMapped = nullptr;
    }
}

// -----------------------------------------------------------------------------

void ProcessJoystickAdded( SDL_JoyDeviceEvent DeviceEvent )
{
    // access the joystick
    SDL_Joystick* NewJoystick = SDL_JoystickOpen( DeviceEvent.which );
    
    if( !NewJoystick )
      return;
    
    // find out joystick instance ID and GUID
    SDL_JoystickGUID NewGUID = SDL_JoystickGetGUID( NewJoystick );
    Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
    
    // update the list of connected joysticks
    ConnectedJoysticks[ AddedInstanceID ] = NewGUID;
}

// -----------------------------------------------------------------------------

void ProcessJoystickRemoved( SDL_JoyDeviceEvent DeviceEvent )
{
    // find out joystick instance ID and GUID
    Sint32 RemovedInstanceID = DeviceEvent.which;
    SDL_Joystick* OldJoystick = SDL_JoystickFromInstanceID( RemovedInstanceID );
    
    // update the list of connected joysticks
    ConnectedJoysticks.erase( RemovedInstanceID );
    SDL_JoystickClose( OldJoystick );
}


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


// use this funcion to get the executable path
// in a portable way (can't be done without libraries)
string GetProgramFolder()
{
    if( SDL_Init( 0 ) )
      THROW( "cannot initialize SDL" );
    
    char* SDLString = SDL_GetBasePath();
    string Result = SDLString;
    
    SDL_free( SDLString );
    SDL_Quit();
    
    return Result;
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main()
{
	try
	{
        // log to the program folder
        ProgramFolder = GetProgramFolder();
        LOG_TO_FILE( ProgramFolder + "DebugLog" );
        
        // init SDL
        LOG( "Initializing SDL" );
        
        Uint32 SDLSubsystems =
        (
            SDL_INIT_VIDEO      |
            SDL_INIT_TIMER      |
            SDL_INIT_EVENTS     |
            SDL_INIT_JOYSTICK
        );
        
        if( SDL_Init( SDLSubsystems ) != 0 )
          THROW( string("Cannot initialize SDL: ") + SDL_GetError() );
        
        // enable SDL joystick events
        SDL_JoystickEventState( SDL_ENABLE );
        
        // open all connected joysticks
        int NumberOfJoysticks = SDL_NumJoysticks();
        LOG( "Active joysticks: " + to_string( NumberOfJoysticks ) );
        
        for( int Joystick = 0; Joystick < NumberOfJoysticks; Joystick++ )
        {
            SDL_Joystick* NewJoystick = SDL_JoystickOpen( Joystick );
            
            if( NewJoystick )
            {
                Sint32 AddedInstanceID = SDL_JoystickInstanceID( NewJoystick );
                SDL_JoystickGUID AddedGUID = SDL_JoystickGetGUID( NewJoystick );
                ConnectedJoysticks[ AddedInstanceID ] = AddedGUID;
            }
        }
        
        // we need to create a window for SDL to receive any events
        OpenGL2D.CreateOpenGLWindow();
        
        if( !OpenGL2D.Window )
          THROW( string("Window cannot be created: ") + SDL_GetError() );
        
        // =======================
        
        // Setup Dear ImGui context
        LOG( "Starting imgui" );
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        // configure ImGui
        ImGui::StyleColorsLight();
        io.FontGlobalScale = 1.0;
        io.IniFilename = NULL;
        
        // add Spanish characters to ImGui
        ImVector< ImWchar > GlyphRanges;
        ImFontGlyphRangesBuilder GlyphBuilder;
        GlyphBuilder.AddRanges( ImGui::GetIO().Fonts->GetGlyphRangesDefault() );  // add all the standard characters
        GlyphBuilder.AddText( u8"\u00E1\u00E9\u00ED\u00F3\u00FA\u00C1\u00C9\u00CD\u00D3\u00DA\u00FC\u00DC\u00F1\u00D1\u00A1\u00BF" );  // add all specific Spanish characters
        GlyphBuilder.BuildRanges( &GlyphRanges );
        
        // ImGui needs to use a custom font to render non-default characters
        string FontPath = ProgramFolder + "GuiFont.ttf";
        ImGui::GetIO().Fonts->AddFontFromFileTTF( FontPath.c_str(), 15, NULL, GlyphRanges.Data );
        ImGui::GetIO().Fonts->Build();
        
        // ImGui needs to use different shader versions
        // depending on the platform and OpenGL context
        #if defined( __arm__ ) 
          #define IMGUI_IMPL_OPENGL_ES2
          const char* glsl_version = "#version 100";
        #elif defined( __APPLE__ )
          const char* glsl_version = "#version 150";
        #else
          const char* glsl_version = "#version 130";
        #endif
        
        // Setup ImGui Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL( OpenGL2D.Window, OpenGL2D.OpenGLContext );
        ImGui_ImplOpenGL3_Init( glsl_version );
        
        // initialize languages
        Languages[ "English" ] = LanguageEnglish;
        Languages[ "Spanish" ] = LanguageSpanish;
        
        // load our gamepad texture
        GamepadTextureID = LoadTexture( ProgramFolder + "Images" + PathSeparator + "GamepadMapping.png" );
        
        // load our configuration from XML file
        LoadControls( ProgramFolder + "Config-Controls.xml" );
        
        // initialize the window
        SDL_SetWindowTitle( OpenGL2D.Window, "Vircon32: Edit Controls" );
        SDL_SetWindowPosition( OpenGL2D.Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
        
        // on non-windows systems load and set the window icon
        // (not needed on Windows: already packed in the executable)
        // but do not crash just because the icon is not found
        #if !defined(__WIN32__) && !defined(_WIN32) && !defined(_WIN64)            
          try
          {
              string IconPath = string(ProgramFolder) + "Images" + PathSeparator + "EditControlsMultisize.ico";
              SDL_Surface* WindowIcon = IMG_Load( IconPath.c_str() );
              SDL_SetWindowIcon( OpenGL2D.Window, WindowIcon );
              LOG( "Loaded program icon" );
          }
          catch( exception& e )
          {
              LOG( "Cannot set window icon: " + string(e.what()) );
          }
        #endif
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Main loop 
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        LOG( "Starting main loop" );
        GlobalLoopActive = true;
        
        // begin message loop
        while( GlobalLoopActive )
        {
            // process window events
            SDL_Event Event;
            
            while( SDL_PollEvent( &Event ) )
            {
                // respond to the global quit event
                if( Event.type == SDL_QUIT )
                  if( ShowMessageBoxYesNo( Texts(TextIDs::Dialogs_AreYouSure), Texts(TextIDs::Dialogs_ExitNoSave_Label) ) )
                    GlobalLoopActive = false;
                
                // let ImGui process keyboard events
                ImGui_ImplSDL2_ProcessEvent( &Event );
                
                // process keyboard events
                if( Event.type == SDL_KEYDOWN )
                  ProcessKeyboardKeyDown( Event.key );
                
                // respond to joysticks being added or removed
                if( Event.type == SDL_JOYDEVICEADDED )
                  ProcessJoystickAdded( Event.jdevice );
                
                if( Event.type == SDL_JOYDEVICEREMOVED )
                  ProcessJoystickRemoved( Event.jdevice );
                
                // process joystick events
                if( Event.type == SDL_JOYBUTTONDOWN )
                  ProcessJoystickButtonDown( Event.jbutton );
                
                if( Event.type == SDL_JOYAXISMOTION )
                  ProcessJoystickAxisMotion( Event.jaxis );
                
                if( Event.type == SDL_JOYHATMOTION )
                  ProcessJoystickHatMotion( Event.jhat );
            }
            
            // Render GUI (full screen)
            RenderGUI();
            
            // Show updates on screen
            SDL_GL_SwapWindow( OpenGL2D.Window );
        }
        
        // delete all joystick profiles
        for( auto Pair: JoystickProfiles )
          delete Pair.second;
        
        // close all connected joysticks
        for( auto Pair: ConnectedJoysticks )
        {
            SDL_Joystick* ClosedJoystick = SDL_JoystickFromInstanceID( Pair.first );
            SDL_JoystickClose( ClosedJoystick );
        }
        
        // shut down imgui
        LOG( "Shutting down imgui" );
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        
        // clean-up in reverse order
        LOG( "Exiting" );
        ReleaseTexture( GamepadTextureID );
        OpenGL2D.DestroyOpenGLWindow();
        SDL_Quit();
	}
    
    catch( const exception& e )
    {
        LOG( "ERROR: " + string(e.what()) );
        string ErrorMessage = Texts(TextIDs::Errors_TopLevel_Label) + string(e.what());
        
        SDL_ShowSimpleMessageBox
        (
            SDL_MESSAGEBOX_ERROR,
            Texts(TextIDs::Errors_TopLevel_Title),
            ErrorMessage.c_str(),
            nullptr
        );
    }
}
