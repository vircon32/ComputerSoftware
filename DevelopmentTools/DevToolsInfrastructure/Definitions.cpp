// *****************************************************************************
    // include project headers
    #include "Definitions.hpp"
// *****************************************************************************


// =============================================================================
//      MATH - MISCELLANEOUS FUNCTIONS
// =============================================================================


float Min( float x, int y )  { return x < y? x : y; }
float Min( int x, float y )  { return x < y? x : y; }
float Max( float x, int y )  { return x > y? x : y; }
float Max( int x, float y )  { return x > y? x : y; }
