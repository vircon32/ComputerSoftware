// *****************************************************************************
    // include project headers
    #include "VirconEmulator.hpp"
    #include "StopWatch.hpp"
    #include "OpenGL2DContext.hpp"
    #include "UserActions.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>           // [ C++ STL ] I/O Streams
    
    // include SDL2 headers
    #include <SDL2/SDL.h>         // [ SDL2 ] Main header
    
    // include OpenAL headers
    #if defined(__APPLE__)
      #include <OpenAL/al.h>      // [ OpenAL ] Main header
      #include <AL/alut.h>        // [ OpenAL ] Utility Toolkit
    #else
      #include <AL/al.h>          // [ OpenAL ] Main header
      #include <AL/alut.h>        // [ OpenAL ] Utility Toolkit
    #endif
    
    // on Linux, include GTK headers
    #if defined(__linux__)
      #include <gtk/gtk.h>        // [ GTK ] Main header
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char* Arguments[] )
{
    try
    {
        // init SDL
        cout << "Initializing SDL" << endl;
        
        Uint32 SDLSubsystems =
        (
            SDL_INIT_VIDEO      |
            SDL_INIT_AUDIO      |
            SDL_INIT_TIMER      |
            SDL_INIT_EVENTS
        );
        
        if( SDL_Init( SDLSubsystems ) != 0 )
          throw runtime_error( "Cannot initialize SDL" );
        
        // we need to create a window for SDL to receive any events
        OpenGL2D.CreateOpenGLWindow();
        
        // =======================
        
        // set alpha blending
        cout << "Enabling alpha blending" << endl;
        glEnable( GL_BLEND );
        OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
        
        //initialize audio
        cout << "Initializing audio" << endl;
        alutInit( NULL, NULL );
        
        // locating listener
        alListener3f( AL_POSITION, 0, 0, 0 );
        alListenerf( AL_GAIN, 1.0 );
        
        // on linux, initialize GTK
        #if defined(__linux__)
          cout << "Initializing GTK" << endl;
          gtk_init( &NumberOfArguments, &Arguments );
        #endif
        
        // -----------------------------------------------------------------------------
        
        // load the standard bios from the emulator's local bios folder
        Vircon.LoadBios( "./Bios/StandardBios.v32" );
        
        // turn on Vircon VM
        Vircon.Initialize();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // program state control
        cout << "Starting main loop" << endl;
        bool WindowActive = true;
        GlobalLoopActive = true;
        float PendingFrames = 1;
        
        // timing control
        StopWatch Watch;
        
        // begin message loop
        while( GlobalLoopActive )
        {
            // process window events
            SDL_Event Event;
            
            while( SDL_PollEvent( &Event ) )
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
                        WindowActive = false;
                        Vircon.Pause();
                    }
                    
                    // on these cases, window updates are resumed
                    if( Event.window.event == SDL_WINDOWEVENT_RESTORED
                    ||  Event.window.event == SDL_WINDOWEVENT_SHOWN
                    ||  Event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )   
                    {
                        WindowActive = true;
                        Vircon.Resume();
                    }
                    
                    // on this case, window should be redrawn
                    if( Event.window.event == SDL_WINDOWEVENT_EXPOSED )
                      OpenGL2D.RenderFrame();
                    
                    // on any window event (such as lose focus) "stop time"
                    Watch.GetStepTime();
                }
                
                // respond to keys being pressed
                if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
                {
                    SDL_Keycode Key = Event.key.keysym.sym;
                    
                    // Escape key toggles emulator pause
                    if( Key == SDLK_ESCAPE )
                    {
                        WindowActive = !WindowActive;
                        
                        if( WindowActive ) Vircon.Resume();
                        else Vircon.Pause();
                    }
                    
                    // Key F5 resets the machine
                    if( Key == SDLK_F5 ) Vircon.Reset();
                    
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
                            if( Vircon.PowerIsOn )
                              Vircon.PowerOff();
                            else
                              Vircon.PowerOn();
                        }
                        
                        // CTRL+R = Reset
                        if( Key == SDLK_r )
                          Vircon.Reset();
                        
                        // Ctrl+L = Load cartridge
                        if( Key == SDLK_l )
                          UserActions::LoadCartridge();
                        
                        // Ctrl+U = Unload cartridge
                        if( Key == SDLK_u )
                          UserActions::UnloadCartridge();
                        
                        // Ctrl+I = Load memory card
                        if( Key == SDLK_i )
                        {
                            if( Vircon.HasMemoryCard() )
                              UserActions::LoadMemoryCard();
                        };
                        
                        // Ctrl+O = Unload memory card
                        if( Key == SDLK_o )
                        {
                            if( Vircon.HasMemoryCard() )
                              UserActions::UnloadMemoryCard();
                        }
                        
                        // Ctrl+F = Fullscreen toggle
                        if( Key == SDLK_f )
                        {
                            if( OpenGL2D.FullScreen ) OpenGL2D.ExitFullScreen();  
                            else OpenGL2D.SetFullScreen();
                        }
                        
                        // Ctrl+M = Mute toggle
                        if( Key == SDLK_m )
                          Vircon.SetMute( !Vircon.IsMuted() );
                    }
                }
                
                // - - - - - - - - - - - - - - - - - - - - - - - - - -
                // NOW, LET MACHINE REACT TO THIS MESSAGE
                // (but while window is inactive, events will get ignored)
                
                if( WindowActive )
                  Vircon.ProcessEvent( Event );
            }
            
            // update frame only when needed
            if( !WindowActive ) continue;
            
            // measure cycle time
            double TimeStep = Watch.GetStepTime();
            PendingFrames += TimeStep * 60.0;
            if( PendingFrames < 0.9 ) continue;
            
            while( PendingFrames >= 0.9 )
            {
                // run another frame
                Vircon.RunNextFrame();
                
                // this frame is done
                PendingFrames = max( PendingFrames - 1, 0.0f );
            }
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // THE FOLLOWING WILL BE DONE JUST ONCE PER UPDATE
            
            // (1) Show updates on screen
            OpenGL2D.RenderFrame();
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // turn off Vircon VM
        Vircon.Terminate();
        
        // shut down ALUT
        cout << "Terminating audio" << endl;
        alutExit();
        
        // clean-up in reverse order
        cout << "Exiting" << endl;
        OpenGL2D.DestroyOpenGLWindow();
        SDL_Quit();
    }
    
    catch( const exception& e )
    {
        // report the error and signal abnormal termination
        cout << "ERROR: " << e.what() << endl;
        return 1;
    }
    
    // signal a successful termination
    return 0;
}
