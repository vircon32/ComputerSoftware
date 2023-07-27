// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "AudioOutput.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


/* -------------------------------------------------------------------------- //
    THREAD SAFETY CONSIDERATIONS:
    -------------------------------
    (1) Thread needs synchronization to access AudioOutput instance variables
    (2) Any exceptions thrown need to be caught, since they cannot trespass
        the boundary to the main thread
// -------------------------------------------------------------------------- */


// =============================================================================
//      THREAD FUNCTION FOR BACKGROUND CONTINUOUS PLAY
// =============================================================================


int AudioPlaybackThread( void* Parameters )
{
    // thread exit code defaults to success
    int ExitCode = 0;
    
    // (1) obtain class instance from parameters
    if( !Parameters )
    {
        LOG( "Audio thread: Audio output instance not received" );
        return 1;
    }
    
    AudioOutput* AudioInstance = (AudioOutput*)Parameters;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    try
    {
        // (2) keep thread alive until audio output stops it
        while( !AudioInstance->ThreadExitFlag )
        {
            // (2.1) if not paused, update sound buffers
            if( !AudioInstance->ThreadPauseFlag )
            {
                AudioInstance->QueueFilledBuffers();
                AudioInstance->UnqueuePlayedBuffers();
            }
            
            // (2.2) when idle, sleep and re-check playback state periodically
            SDL_Delay( 5 );
        }
        
        LOG( "Audio thread exiting" );
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //   THREAD TERMINATION HANDLING
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    catch( const exception& e )
    {
        // log in thread
        LOG( "[EXCEPTION]: In audio thread: " + string(e.what()) );
        
        // store exception message to treat it in the main thread
        // (necessary since exceptions do not cross threads)
        AudioInstance->ThreadErrorMessage = e.what();
        
        // provide an error exit code
        ExitCode = 1;
    }
    
    catch( ... )
    {
        // store exception message to treat it in the main thread
        LOG( "[EXCEPTION]: In audio thread: Unknown exception happened" );
        
        // provide an error exit code
        ExitCode = 2;
    }
    
    // provide exit code
    return ExitCode;
}
