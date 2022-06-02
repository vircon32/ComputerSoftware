// *****************************************************************************
    // include project headers
    #include "Definitions.hpp"
// *****************************************************************************


// =============================================================================
//      MATH - MISCELLANEOUS FUNCTIONS
// =============================================================================


bool IsPowerOf2( unsigned Value )
{
    return (Value > 0) && !(Value & (Value - 1));
}

unsigned NextPowerOf2( unsigned Value )
{
    unsigned PowerOf2 = 1;
    
    while( PowerOf2 < Value )
      PowerOf2 <<= 1;
    
    return PowerOf2;
}

float Min( float x, int y )  { return x < y? x : y; }
float Min( int x, float y )  { return x < y? x : y; }
float Max( float x, int y )  { return x > y? x : y; }
float Max( int x, float y )  { return x > y? x : y; }
