// *****************************************************************************
    // include project headers
    #include "StopWatch.hpp"
// *****************************************************************************


// =============================================================================
//      STEP TIMER - CLASS IMPLEMENTATION
// =============================================================================


StopWatch::StopWatch()
{
    // get frequency for SDL's high resolution counter
    // (in counts per second)
    CounterFrequency = SDL_GetPerformanceFrequency();
    
    // obtain initial counter value
    LastTimestamp = SDL_GetPerformanceCounter();
}

// -----------------------------------------------------------------------------

double StopWatch::LastAbsoluteTime()
{
    // convert last stored counter to time units
    return double( LastTimestamp / (double)CounterFrequency );
}

// -----------------------------------------------------------------------------

double StopWatch::GetStepTime()
{
    // preserve previous count
    Uint64 PreviousTimestamp = LastTimestamp;
    
    // obtain current count (it's stored for next call to this function)
    LastTimestamp = SDL_GetPerformanceCounter();
    
    // convert cont difference to actual time
    // NOTE: LARGE_INTEGER is a union, so for arithmetic we choose its largest member
    return double( (LastTimestamp - PreviousTimestamp) / (double)CounterFrequency );
}

// -----------------------------------------------------------------------------

double StopWatch::GetResolution()
{
    Uint64 InitialCount;
    Uint64   FinalCount;
    
    double TotalTime = 0;
    
    // take the average of 10 times for more precision
    for( int i = 1; i < 10; i++ )
    {
        // get current count
        InitialCount = SDL_GetPerformanceCounter();
          FinalCount = SDL_GetPerformanceCounter();
        
        // keep querying counter until count advances
        while( FinalCount == InitialCount )
          FinalCount = SDL_GetPerformanceCounter();
        
        // add difference (in seconds) to accumulator
        TotalTime += double( FinalCount - InitialCount ) / CounterFrequency;
    }
    
    // return average time (in seconds)
    return TotalTime / 10;
}
