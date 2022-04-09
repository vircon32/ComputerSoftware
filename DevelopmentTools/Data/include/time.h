/* *****************************************************************************
*  Vircon32 standard library: "time.h"            File version: 2022/02/25     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all definitions and functions needed by the programs to  *
*  interact with Vircon timer. This includes functions to read current time    *
*  and date, and functions to enable programs to synchronize time. In addition *
*  to that, some utility functions allow to convert time and date to the usual *
*  human-readable formats.                                                     *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef TIME_H
    #define TIME_H
// *****************************************************************************


// =============================================================================
//   GENERAL TIMING DEFINITIONS
// =============================================================================


// timing properties of the console
#define frames_per_second   60
#define frame_time  (1.0 / frames_per_second)


// =============================================================================
//   READING TIME INFORMATION
// =============================================================================


// returned number is the number of CPU cycles elapsed
// within the current frame (however, note that Vircon
// emulators will make no guarantees on accurate timing
// of the cycles within a same frame)
int get_cycle_counter()
{
    asm
    {
        "in R0, TIM_CycleCounter"
    }
}

// -----------------------------------------------------------------------------

// returned number is the number of frames elapsed from
// the last time the console was switched on or reset
int get_frame_counter()
{
    asm
    {
        "in R0, TIM_FrameCounter"
    }
}

// -----------------------------------------------------------------------------

// returned value is a compound that contains both
// the current year and the days elapsed within it;
// these can be translated to usual date formats with
// functions date_year, date_month and date_day
int get_date()
{
    asm
    {
        "in R0, TIM_CurrentDate"
    }
}

// -----------------------------------------------------------------------------

// returned value is the number of seconds elapsed
// from the beginning of the current day at 00:00;
// these can be translated to usual time formats with
// functions time_hours, time_minutes and time_seconds
int get_time()
{
    asm
    {
        "in R0, TIM_CurrentTime"
    }
}


// =============================================================================
//   TRANSLATING DATE AND TIME
// =============================================================================


// structure with the usual date format
//  - year: the year AD (years BC are not supported)
//  - month: from 1 (January) to 12 (December)
//  - day: starting from 1
struct date_info
{
    int year;
    int month;
    int day;
};

// -----------------------------------------------------------------------------

// converts a Vircon date value to the usual human
// readable format separating year, month and day
void translate_date( int date, date_info* translated )
{
    // decompose date value
    translated->year = date >> 16;
    int days_in_year = date & 0x0000FFFF;
    
    // define the days of each month
    int[ 12 ] month_days = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    
    // take leap years into account
    bool is_leap_year = ((translated->year % 4) == 0) && ((translated->year % 100) != 0);
    if( is_leap_year ) month_days[ 1 ] = 29;
    
    // determine month and day
    translated->month = 1;
    
    // accumulate elapsed days up to November
    for( int m = 0; m < 11; ++m )
    {
        if( days_in_year < month_days[ m ] )
        {
            translated->day = days_in_year + 1;
            return;
        }
        
        days_in_year -= month_days[ m ];
        translated->month++;
    }
    
    // if this is reached, month must be December
    translated->month = 12;
    translated->day = days_in_year + 1;
}

// -----------------------------------------------------------------------------

// structure with the usual time format
//  - hours: in range 0 to 23
//  - minutes: in range 0 to 59
//  - seconds: in range 0 to 59
struct time_info
{
    int hours;
    int minutes;
    int seconds;
};

// -----------------------------------------------------------------------------

// converts a Vircon time value to the usual human
// readable format separating hours, minutes and seconds
//  - hours: in range 0 to 23
//  - minutes: in range 0 to 59
//  - seconds: in range 0 to 59
void translate_time( int time, time_info* translated )
{
    translated->hours = time / 3600;
    translated->minutes = (time % 3600) / 60;
    translated->seconds = time % 60;
}


// =============================================================================
//   TIMING CONTROL FUNCTIONS
// =============================================================================


// waits for the current frame to end
// (CPU blocks until next frame begins)
void end_frame()
{
    asm{ "wait" }
}

// -----------------------------------------------------------------------------

// waits for the specified number of frames
// (parameter is not in seconds as in regular C)
void sleep( int frames )
{
    int initial_frames = get_frame_counter();
    int final_frames = initial_frames + frames;
    
    while( get_frame_counter() < final_frames )
      end_frame();
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
