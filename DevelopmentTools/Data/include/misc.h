/* *****************************************************************************
*  Vircon32 standard library: "misc.h"            File version: 2021/02/18     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file allows programs to use Vircon CPU's memory operations and random  *
*  number generation capabilities. It also contains other miscellaneous or     *
*  general use functions that don't match the scope of other headers.          *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef MISC_H
    #define MISC_H
// *****************************************************************************


// =============================================================================
//   MEMORY FUNCTIONS
// =============================================================================


void memset( void* destination, int value, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {destination}"
        "mov SR, {value}"
        "sets"
    }
}

// -----------------------------------------------------------------------------

void memcpy( void* destination, void* source, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {destination}"
        "mov SR, {source}"
        "movs"
    }
}

// -----------------------------------------------------------------------------

int memcmp( void* region1, void* region2, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {region1}"
        "mov SR, {region2}"
        "cmps R0"
    }
}


// =============================================================================
//   RANDOM NUMBER GENERATION
// =============================================================================


int rand()
{
    asm
    {
        "in R0, RNG_CurrentValue"
    }
}

// -----------------------------------------------------------------------------

// because of limitations of our specific generator, value
// 0 will never be actually set as seed (request is ignored)
void srand( int seed )
{
    asm
    {
        "mov R0, {seed}"
        "out RNG_CurrentValue, R0"
    }
}


// =============================================================================
//   PROGRAM FLOW
// =============================================================================


// since there are no exit codes from main, there
// is no need to also provide C's "abort" function
void exit()
{
    asm
    {
        "hlt"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
