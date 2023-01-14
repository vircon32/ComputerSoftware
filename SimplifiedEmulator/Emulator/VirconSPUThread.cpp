// *****************************************************************************
    // include project headers
    #include "VirconSPU.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


/* -------------------------------------------------------------------------- //
    THREAD SAFETY CONSIDERATIONS:
    -------------------------------
    (1) Thread needs synchronization to access VirconSPU instance variables
    (2) Any exceptions thrown need to be caught, since they cannot trespass
        the boundary to the main thread
// -------------------------------------------------------------------------- */


// =============================================================================
//      THREAD FUNCTION FOR BACKGROUND CONTINUOUS PLAY
// =============================================================================


int SPUPlaybackThread( void* Parameters )
{
    // thread exit code defaults to success
    int ExitCode = 0;
    
    // (1) obtain class instance from parameters
    if( !Parameters )
    {
        throw runtime_error( "Vircon SPU instance not received" );
        return 1;
    }
    
    VirconSPU* SPUInstance = (VirconSPU*)Parameters;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    try
    {
        // (2) keep thread alive until the SPU stops it
        while( !SPUInstance->ThreadExitFlag )
        {
            // (2.1) if not paused, update sound buffers
            if( !SPUInstance->ThreadPauseFlag )
            {
                SPUInstance->QueueFilledBuffers();
                SPUInstance->UnqueuePlayedBuffers();
            }
            
            // (2.2) when idle, sleep and re-check playback state periodically
            SDL_Delay( 5 );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //   THREAD TERMINATION HANDLING
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    catch( const exception& e )
    {
        // log in thread
        cout << "[EXCEPTION]: " << e.what() << endl;
        
        // store exception message to treat it in the main thread
        // (necessary since exceptions do not cross threads)
        SPUInstance->ThreadErrorMessage = e.what();
        
        // provide an error exit code
        ExitCode = 1;
    }
    
    catch( ... )
    {
        // store exception message to treat it in the main thread
        cout << "[EXCEPTION]: Unknown exception happened" << endl;
        
        // provide an error exit code
        ExitCode = 2;
    }
    
    // provide exit code
    return ExitCode;
}
