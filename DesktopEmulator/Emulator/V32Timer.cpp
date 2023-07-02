// *****************************************************************************
    // include project headers
    #include "V32Timer.hpp"
    
    // include C/C++ headers
    #include <time.h>           // [ ANSI C ] Date and time
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: V32 TIMER
// =============================================================================


V32Timer::V32Timer()
{
    // obtain current time
    time_t CreationTime;
    time( &CreationTime );
    struct tm* CreationTimeInfo = localtime( &CreationTime );
    
    // store current date as (year|days)
    // (Careful! C gives year counting from 1900)
    CurrentDate = ((CreationTimeInfo->tm_year+1900) << 16) | CreationTimeInfo->tm_yday;
    
    // store current time as seconds within this day
    CurrentTime = CreationTimeInfo->tm_hour * 3600
                + CreationTimeInfo->tm_min * 60
                + CreationTimeInfo->tm_sec;
}

// -----------------------------------------------------------------------------

bool V32Timer::ReadPort( int32_t LocalPort, VirconWord& Result )
{
    // check range
    if( LocalPort > CLK_LastPort )
      return false;
      
    // provide value (for efficiency, do the checks
    // starting by the most frequently accessed ports)
    if( LocalPort == (int32_t)CLK_LocalPorts::FrameCounter )
      Result.AsInteger = FrameCounter;
      
    else if( LocalPort == (int32_t)CLK_LocalPorts::CycleCounter )
      Result.AsInteger = CycleCounter;
      
    else if( LocalPort == (int32_t)CLK_LocalPorts::CurrentTime )
      Result.AsInteger = CurrentTime;
      
    else
      Result.AsInteger = CurrentDate;
    
    return true;
}

// -----------------------------------------------------------------------------

bool V32Timer::WritePort( int32_t LocalPort, VirconWord Value )
{
    // ignore write request (all these registers are read-only)
    return false;
}

// -----------------------------------------------------------------------------

void V32Timer::RunNextCycle()
{
    CycleCounter++;
}

// -----------------------------------------------------------------------------

void V32Timer::ChangeFrame()
{
    CycleCounter = 0;
    FrameCounter++;
    
    // current time advances each second
    if( (FrameCounter % 60) == 0)
      CurrentTime++;
    
    // current date advances each day
    if( CurrentTime >= 86400 )
    {
        CurrentTime = 0;
        CurrentDate++;
        
        // check if we should change year
        int Year = CurrentDate >> 16;
        bool IsLeapYear = ( ((Year % 4) == 0) && ((Year % 100) != 0) );
        int DaysThisYear = (IsLeapYear? 366 : 365);
        int Days = CurrentDate & 0xFFFF;
        
        // if needed, advance year and reset days to 0
        if( Days >= DaysThisYear )
          CurrentDate = (Year+1) << 16;
    }
}

// -----------------------------------------------------------------------------

void V32Timer::Reset()
{
    CycleCounter = 0;
    FrameCounter = 0;
}
