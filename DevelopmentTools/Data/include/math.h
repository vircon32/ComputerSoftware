/* *****************************************************************************
*  Vircon32 standard library: "math.h"            File version: 2021/02/18     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all definitions and functions needed by the programs to  *
*  use Vircon CPU math capabilities, as well as to operate with numbers in     *
*  general. This includes integer and float arithmetic, comparison functions,  *
*  trigonometry and exponential operations.                                    *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef MATH_H
    #define MATH_H
// *****************************************************************************


// =============================================================================
//   GENERAL DEFINITIONS
// =============================================================================


// pi is used for angles and trigonometry
#define pi 3.1415926

// limits for built-in data types
#define INT_MIN  0x80000000   // careful! C parsers will not take "-2147483648" as a single entity
#define INT_MAX  2147483647


// =============================================================================
//   ARITHMETIC FUNCTIONS
// =============================================================================


float fmod( float x, float y )
{
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, {y}"
        "fmod R0, R1"
        "pop R1"
    }
}


// =============================================================================
//   COMPARISON FUNCTIONS
// =============================================================================


int min( int a, int b )
{
    asm
    {
        "push R1"
        "mov R0, {a}"
        "mov R1, {b}"
        "imin R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

int max( int a, int b )
{
    asm
    {
        "push R1"
        "mov R0, {a}"
        "mov R1, {b}"
        "imax R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

int abs( int a )
{
    asm
    {
        "mov R0, {a}"
        "iabs R0"
    }
}

// -----------------------------------------------------------------------------

float fmin( float x, float y )
{
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, {y}"
        "fmin R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

float fmax( float x, float y )
{
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, {y}"
        "fmax R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

float fabs( float x )
{
    asm
    {
        "mov R0, {x}"
        "fabs R0"
    }
}


// =============================================================================
//   FLOAT ROUNDING FUNCTIONS
// =============================================================================


float floor( float x )
{
    asm
    {
        "mov R0, {x}"
        "flr R0"
    }
}

// -----------------------------------------------------------------------------

float ceil( float x )
{
    asm
    {
        "mov R0, {x}"
        "ceil R0"
    }
}

// -----------------------------------------------------------------------------

float round( float x )
{
    asm
    {
        "mov R0, {x}"
        "round R0"
    }
}


// =============================================================================
//   TRIGONOMETRIC FUNCTIONS
// =============================================================================


// input angle is given in radians
float sin( float angle )
{
    asm
    {
        "mov R0, {angle}"
        "sin R0"
    }
}

// -----------------------------------------------------------------------------

// input angle is given in radians
float cos( float angle )
{
    // computes cos(x) as sin(x+pi/2)
    asm
    {
        "mov R0, {angle}"
        "fadd R0, 1.570796"
        "sin R0"
    }
}

// -----------------------------------------------------------------------------

// input angle is given in radians
// not defined when cos(angle) = 0
// (will trigger a hardware error)
float tan( float angle )
{
    // computes tan(x) as sin(x)/cos(x)
    // = sin(x) / sin(x+pi/2)
    asm
    {
        "push R1"
        "mov R1, {angle}"
        "mov R0, R1"
        "sin R0"
        "fadd R1, 1.570796"
        "sin R1"
        "fdiv R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

// result angle is returned in radians
// not defined if abs(x) > 1
// (will trigger a hardware error)
float asin( float x )
{
    // computes asin(x) = -acos(x) + pi/2
    asm
    {
        "mov R0, {x}"
        "acos R0"
        "fsgn R0"
        "fadd R0, 1.570796"
    }
}

// -----------------------------------------------------------------------------

// result angle is returned in radians
// not defined if abs(x) > 1
// (will trigger a hardware error)
float acos( float x )
{
    asm
    {
        "mov R0, {x}"
        "acos R0"
    }
}

// -----------------------------------------------------------------------------

// result angle is returned in radians
// not defined if x = 0 and y = 0
// (will trigger a hardware error)
float atan2( float x, float y )
{
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, {y}"
        "atan2 R0, R1"
        "pop R1"
    }
}


// =============================================================================
//   EXPONENTIAL FUNCTIONS
// =============================================================================


// not defined for x < 0
// (will trigger a hardware error)
float sqrt( float x )
{
    // computes it as x^0.5
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, 0.5"
        "pow R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

// not defined if x < 0 and y is non integer
// (will trigger a hardware error)
float pow( float x, float y )
{
    asm
    {
        "push R1"
        "mov R0, {x}"
        "mov R1, {y}"
        "pow R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

float exp( float x )
{
    asm
    {
        "push R1"
        "mov R0, 2.718282"
        "mov R1, {x}"
        "pow R0, R1"
        "pop R1"
    }
}

// -----------------------------------------------------------------------------

// not defined for x <= 0
// (will trigger a hardware error)
float log( float x )
{
    asm
    {
        "mov R0, {x}"
        "log R0"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
