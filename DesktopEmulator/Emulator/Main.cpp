// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    #include "DesktopInfrastructure/FilePaths.hpp"
    
    // include emulator headers
    #include "EmulatorControl.hpp"
    #include "GamepadsInput.hpp"
    #include "VideoOutput.hpp"
    #include "AudioOutput.hpp"
    #include "GUI.hpp"
    #include "Settings.hpp"
    #include "Globals.hpp"
    #include "Languages.hpp"
    #include "StopWatch.hpp"
    #include "Texture.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <cstddef>          // [ ANSI C ] Standard definitions
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
    
    // include imgui headers
    #include <imgui/imgui.h>                // [ Dear ImGui ] Main header
    #include <imgui/imgui_impl_sdl.h>       // [ Dear ImGui ] SDL2 backend header
    #include <imgui/imgui_impl_opengl3.h>   // [ Dear ImGui ] OpenGL 3 backend header
    
    // on Linux, include GTK headers
    #if defined(__linux__)
      #include <gtk/gtk.h>      // [ GTK ] Main header
    #endif
    
    // on Windows include headers for unicode conversion
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
      #include <windows.h>      // [ WINDOWS ] Main header
      #include <shellapi.h>     // [ WINDOWS ] Shell API
    #endif
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      BASIC ABI ASSERTIONS
// =============================================================================


void PerformABIAssertions()
{
    LOG( "Performing ABI assertions" );
    V32Word TestWord = {0};
    
    // determine the correct packing sizes
    if( sizeof(V32Word) != 4 )
      THROW( "ABI check failed: Vircon words are not 4 bytes in size" );
    
    // determine the correct bit endianness: instructions
    TestWord.AsInstruction.OpCode = 0x1;
    
    if( TestWord.AsBinary != 0x04000000 )
      THROW( "ABI check failed: Fields of CPU instructions are not correctly ordered" );
    
    // determine the correct byte endianness
    TestWord.AsColor.R = 0x11;
    TestWord.AsColor.G = 0x22;
    TestWord.AsColor.B = 0x33;
    TestWord.AsColor.A = 0x44;
    
    if( TestWord.AsBinary != 0x44332211 )
      THROW( "ABI check failed: Components GPU colors are not correctly ordered as RGBA" );
}

// -----------------------------------------------------------------------------

void PerformPortAssertions()
{
    LOG( "Performing I/O port assertions" );
    V32GPU TestGPU;
    
    // determine the correct location of ports
    int DetectedGPUPortDistance = (int)((V32Word*)(&TestGPU.DrawingAngle) - (V32Word*)(&TestGPU.Command));
    int ExpectedGPUPortDistance = ((int)GPU_LocalPorts::DrawingAngle - (int)GPU_LocalPorts::Command);
    
    if( DetectedGPUPortDistance != ExpectedGPUPortDistance )
      THROW( "Port check failed: GPU ports are not correctly ordered, or there is padding between them" );
}

// -----------------------------------------------------------------------------

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


int main( int NumberOfArguments, char* Arguments[] )
{
    if( NumberOfArguments > 2 )
    {
        cout << "USAGE: Vircon32 <optional: ROM file>" << endl;
        return 1;
    }
    
    try
    {
        // log to the emulator folder
        EmulatorFolder = GetProgramFolder();
        LOG_TO_FILE( EmulatorFolder + "DebugLog" );
        
        // do this test before anything else
        PerformABIAssertions();
        PerformPortAssertions();
        
        // use this to control initialization ourselves
        InitializeGlobalVariables();
        
        // init SDL
        LOG( "Initializing SDL" );
        
        Uint32 SDLSubsystems =
        (
            SDL_INIT_VIDEO      |
            SDL_INIT_AUDIO      |
            SDL_INIT_TIMER      |
            SDL_INIT_EVENTS     |
            SDL_INIT_JOYSTICK
        );
        
        if( SDL_Init( SDLSubsystems ) != 0 )
          THROW( string("Cannot initialize SDL: ") + SDL_GetError() );
        
        // enable SDL joystick events
        SDL_JoystickEventState( SDL_ENABLE );
        
        // begin connection to SDL joysticks
        Gamepads.OpenAllJoysticks();
        
        // we need to create a window for SDL to receive any events
        Video.CreateOpenGLWindow();
        
        // =======================
        
        // log graphic device info
        string GraphicDeviceVendor = (const char *)glGetString( GL_VENDOR );
        string GraphicDeviceModel = (const char *)glGetString( GL_RENDERER );
        
        LOG( "Graphic device vendor: " + GraphicDeviceVendor );
        LOG( "Graphic device model: " + GraphicDeviceModel );
        
        // =======================
        
        // Setup Dear ImGui context
        LOG( "Starting ImGui" );
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        // configure ImGui
        ImGui::StyleColorsClassic();
        ImGui::GetIO().FontGlobalScale = Video.GetRelativeWindowWidth();
        ImGui::GetIO().IniFilename = NULL;
        
        // add Spanish characters to ImGui
        ImVector< ImWchar > GlyphRanges;
        ImFontGlyphRangesBuilder GlyphBuilder;
        GlyphBuilder.AddRanges( ImGui::GetIO().Fonts->GetGlyphRangesDefault() );  // add all the standard characters
        GlyphBuilder.AddText( u8"\u00E1\u00E9\u00ED\u00F3\u00FA\u00C1\u00C9\u00CD\u00D3\u00DA\u00FC\u00DC\u00F1\u00D1\u00A1\u00BF" );  // add all specific Spanish characters
        GlyphBuilder.BuildRanges( &GlyphRanges );
        
        // ImGui needs to use a custom font to render non-default characters
        string FontPath = EmulatorFolder + "GuiFont.ttf";
        if( !FileExists( FontPath ) ) THROW( "Cannot find GUI font file \"GuiFont.ttf\"" );
        ImGui::GetIO().Fonts->AddFontFromFileTTF( FontPath.c_str(), 16, NULL, GlyphRanges.Data );
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
        ImGui_ImplSDL2_InitForOpenGL( Video.GetWindow(), Video.GetOpenGLContext() );
        ImGui_ImplOpenGL3_Init( glsl_version );
        
        // =======================
        
        // initialize OpenGL shaders and their infrastructure
        Video.InitRendering();
        
        // create a framebuffer object
        Video.CreateFramebuffer();
        Video.RenderToScreen();
        
        // set alpha blending
        LOG( "Enabling alpha blending" );
        glEnable( GL_BLEND );
        Video.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
        
        //initialize audio
        LOG( "Initializing audio" );
        Audio.Initialize();
        
        // initialize languages
        Languages[ "English" ] = LanguageEnglish;
        Languages[ "Spanish" ] = LanguageSpanish;
        
        // load decoration images, but do not crash
        // just because any of them are not found
        try
        {
            // on non-windows systems load and set the window icon
            // (not needed on Windows: already packed in the executable)
            #if !defined(WINDOWS_OS)            
              string IconPath = string(EmulatorFolder) + "Images" + PathSeparator + "Vircon32WindowIcon.bmp";
              SDL_Surface* WindowIcon = SDL_LoadBMP( IconPath.c_str() );
              SDL_SetWindowIcon( Video.GetWindow(), WindowIcon );
              LOG( "Loaded program icon" );
            #endif
            
            // load the no signal image
            NoSignalTexture.Load( string(EmulatorFolder) + "Images" + PathSeparator + "NoSignal.bmp" );
        }
        catch( exception& e )
        {
            LOG( "Cannot set window icon: " + string(e.what()) );
        }
        
        // load our configuration from XML files
        LoadControls( EmulatorFolder + "Config-Controls.xml" );
        LoadSettings( EmulatorFolder + "Config-Settings.xml" );
        
        // initialize the window
        string WindowTitle = string("Vircon32: ") + Texts( TextIDs::Status_NoCartridge );
        SDL_SetWindowTitle( Video.GetWindow(), WindowTitle.c_str() );
        SDL_SetWindowPosition( Video.GetWindow(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
        
        // on linux, initialize GTK
        #if defined(__linux__)
          LOG( "Initializing GTK" );
          gtk_init( &NumberOfArguments, &Arguments );
        #endif
        
        // -----------------------------------------------------------------------------
        
        // turn on Vircon VM
        Emulator.Initialize();
        
        // load the standard bios from the emulator's local bios folder
        Console.LoadBios( EmulatorFolder + "Bios" + PathSeparator + BiosFileName );
        
        // if a cartridge file has been specified, load it
        // (this will also turn on the console)
        if( NumberOfArguments == 2 )
        {
            #if defined(WINDOWS_OS)
            
              // on Windows we can't rely on the arguments received
              // in main: ask Windows for the UTF-16 command line
              wchar_t* CommandLineUTF16 = GetCommandLineW();
              wchar_t** ArgumentsUTF16 = CommandLineToArgvW( CommandLineUTF16, &NumberOfArguments );
              
              // now convert the first program argument to UTF-8
              GUI_LoadCartridge( ToUTF8( ArgumentsUTF16[ 1 ] ) );
              
              LocalFree( ArgumentsUTF16 );
          
            #else
                
              // on Linux/Mac arguments in main are already UTF-8
              GUI_LoadCartridge( Arguments[ 1 ] );
              
            #endif
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // program state control
        LOG( "---------------------------------------------------------------------" );
        LOG( "    Starting main loop" );
        LOG( "---------------------------------------------------------------------" );
        GlobalLoopActive = true;
        bool WindowActive = true;
        float PendingFrames = 1;
        
        // depending on focus changes we will wait for events or
        // just poll them and continue; this pointer controls that
        int (*EventProcessor)(SDL_Event *) = &SDL_PollEvent;
        
        // timing control
        StopWatch Watch;
        
        // begin message loop
        while( GlobalLoopActive )
        {
            // process window events
            SDL_Event Event;
            
            while( EventProcessor( &Event ) )
            {
                // - - - - - - - - - - - - - - - - - - - - - - -
                // FIRST, PROCESS THE GLOBAL BEHAVIORS
                // (THESE ALWAYS SHOULD TAKE PRIORITY)
                
                // respond to the quit event
                if( Event.type == SDL_QUIT )
                {
                    GlobalLoopActive = false;
                }
                
                // respond to window events
                if( Event.type == SDL_WINDOWEVENT )
                {
                    // exit when window is closed
                    if( Event.window.event == SDL_WINDOWEVENT_CLOSE )
                      GlobalLoopActive = false;
                    
                    // on these cases, window updates are paused
                    if( Event.window.event == SDL_WINDOWEVENT_MINIMIZED
                    ||  Event.window.event == SDL_WINDOWEVENT_HIDDEN
                    ||  Event.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
                    {
                        LOG("Focus lost");
                        WindowActive = false;
                        MouseIsOnWindow = false;
                        EventProcessor = &SDL_WaitEvent;
                        Emulator.Pause();
                    }
                    
                    // on these cases, window updates are resumed
                    if( Event.window.event == SDL_WINDOWEVENT_RESTORED
                    ||  Event.window.event == SDL_WINDOWEVENT_SHOWN
                    ||  Event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )   
                    {
                        LOG("Focus gained");
                        WindowActive = true;
                        EventProcessor = &SDL_PollEvent;
                        Emulator.Resume();
                    }
                    
                    // on this case, window should be redrawn
                    if( Event.window.event == SDL_WINDOWEVENT_EXPOSED )
                    {
                        ShowEmulatorWindow();
                        SDL_GL_SwapWindow( Video.GetWindow() );
                    }
                    
                    // keep track of when mouse is inside our window
                    if( Event.window.event == SDL_WINDOWEVENT_ENTER )
                      MouseIsOnWindow = true;
                    
                    if( Event.window.event == SDL_WINDOWEVENT_LEAVE )
                      MouseIsOnWindow = false;
                    
                    // on any window event (such as lose focus) "stop time"
                    Watch.GetStepTime();
                }
                
                // respond to keys being pressed
                if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
                {
                    SDL_Keycode Key = Event.key.keysym.sym;
                    
                    // Escape key toggles showing GUI
                    if( Key == SDLK_ESCAPE )
                      MouseIsOnWindow = !MouseIsOnWindow;
                    
                    // Key F5 resets the machine
                    if( Key == SDLK_F5 ) Emulator.Reset();
                    
                    // Key F2 saves state in the current slot
                    if( Key == SDLK_F2 ) GUI_SaveState();
                    
                    // Key F4 loads state from the current slot
                    if( Key == SDLK_F4 ) GUI_LoadState();
                    
                    // when CTRL is pressed, process keyboard shortcuts
                    bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
                    
                    if( ControlIsPressed )
                    {
                        // CTRL+Q = Quit
                        if( Key == SDLK_q )
                          GlobalLoopActive = false;
                        
                        // CTRL+P = Power toggle
                        if( Key == SDLK_p )
                        {
                            if( Emulator.IsPowerOn() )
                              Emulator.SetPower( false );
                            else
                              Emulator.SetPower( true );
                        }
                        
                        // CTRL+R = Reset
                        if( Key == SDLK_r )
                          Emulator.Reset();
                        
                        // Ctrl+L = Load cartridge (or change it)
                        if( Key == SDLK_l )
                        {
                            // power needs to be off
                            if( !Emulator.IsPowerOn() )
                            {
                                if( Console.HasCartridge() )
                                  GUI_ChangeCartridge();
                                else
                                  GUI_LoadCartridge();
                            }
                        };
                        
                        // Ctrl+U = Unload cartridge
                        if( Key == SDLK_u )
                        {
                            // power needs to be off
                            if( !Emulator.IsPowerOn() )
                              if( Console.HasCartridge() )
                                GUI_UnloadCartridge();
                        }
                        
                        // Ctrl+S = Save Screenshot
                        if( Key == SDLK_s )
                          GUI_SaveScreenshot();
                        
                        // Ctrl+1 = Zoom 1X
                        if( Key == SDLK_1 )
                          SetWindowZoom( 1 );
                        
                        // Ctrl+2 = Zoom 2X
                        if( Key == SDLK_2 )
                          SetWindowZoom( 2 );
                        
                        // Ctrl+3 = Zoom 3X
                        if( Key == SDLK_3 )
                          SetWindowZoom( 3 );
                        
                        // Ctrl+F = Fullscreen
                        if( Key == SDLK_f )
                          SetFullScreen();
                        
                        // Ctrl+M = Mute toggle
                        if( Key == SDLK_m )
                          Audio.SetMute( !Audio.IsMuted() );
                    }
                }
                
                // - - - - - - - - - - - - - - - - - - - - - - - - - -
                // NOW, LET EMULATION REACT TO THIS MESSAGE
                // (but while window is inactive, events will get ignored)
                
                if( WindowActive )
                  Gamepads.ProcessEvent( Event );
            }
            
            // update frame only when needed
            if( !WindowActive ) continue;
            
            // redirect all rendering to emulator's display
            Video.RenderToFramebuffer();
            Video.BeginFrame();
            
            // measure cycle time
            double TimeStep = Watch.GetStepTime();
            
            if( Emulator.IsPowerOn() && !Emulator.IsPaused() )
            {
                // execute frames as needed
                PendingFrames += TimeStep * 60.0;
                if( PendingFrames < 0.9 ) continue;
                
                while( PendingFrames >= 0.9 )
                {
                    // run another frame
                    Emulator.RunNextFrame();
                    PendingFrames -= 1;
                }
            }
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // THE FOLLOWING WILL BE DONE JUST ONCE PER UPDATE
            
            // (1) Show the emulator's display on screen
            ShowEmulatorWindow();
            
            // (2) Render GUI (on full screen, only when needed)
            RenderGUI();
            
            // (3) Show updates on screen
            SDL_GL_SwapWindow( Video.GetWindow() );
            
            // (4) Show message boxes when needed
            ShowDelayedMessageBox();
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // turn off Vircon VM
        Emulator.Terminate();
        
        // save settings
        SaveSettings( EmulatorFolder + "Config-Settings.xml" );
        
        // free scene resources
        LOG( "---------------------------------------------------------------------" );
        LOG( "    Performing terminations" );
        LOG( "---------------------------------------------------------------------" );
        
        // end connection to SDL joysticks
        Gamepads.CloseAllJoysticks();
        
        // shut down audio
        LOG( "Terminating audio" );
        Audio.Terminate();
        
        // shut down imgui
        LOG( "Shutting down ImGui" );
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        
        // clean-up in reverse order
        LOG( "Exiting" );
        Video.Destroy();
        SDL_Quit();
    }
    
    catch( const exception& e )
    {
        LOG( "ERROR: " + string(e.what()) );
        string Message = Texts( TextIDs::Errors_TopLevel_Label ) + string(e.what());
        
        SDL_ShowSimpleMessageBox
        (
            SDL_MESSAGEBOX_ERROR,
            Texts( TextIDs::Errors_TopLevel_Title ),
            Message.c_str(),
            nullptr
        );
    }
    
    // signal a successful termination
    return 0;
}
