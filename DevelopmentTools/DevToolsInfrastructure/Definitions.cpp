// *****************************************************************************
    // include project headers
    #include "Definitions.hpp"
    
    // include C/C++ headers
    #include <sstream>          // [ C++ STL ] String streams
    #include <iomanip>          // [ C++ STL ] I/O Manipulation
    #include <cmath>            // [ ANSI C ] Mathematics
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      MATH - ANGLE RELATED FUNCTIONS
// =============================================================================


const double DegreesPerRadian = 180.0 / PI;

// -----------------------------------------------------------------------------

double CalculateAngle( double DeltaX, double DeltaY )
{
    if( !(DeltaX || DeltaY) )
      throw runtime_error( "CalculateAngle: Attempting 0/0 division" );

    // for edge cases
    if( !DeltaX )   return (DeltaY > 0)?  90 : 270;
    if( !DeltaY )   return (DeltaX > 0)?   0 : 180;
    
    // for all others
    double FirstCuadrantAngle = DegreesPerRadian * atan( DeltaY / DeltaX );
    
    if( DeltaY < 0 )  return (DeltaX < 0)?  (180 - FirstCuadrantAngle) : (      FirstCuadrantAngle);
    else              return (DeltaX < 0)?  (180 + FirstCuadrantAngle) : (360 - FirstCuadrantAngle);
}


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


// =============================================================================
//      MISCELLANEOUS FUNCTIONS
// =============================================================================


bool ListContains( std::list<std::string> Container, std::string Value )
{
	for( auto s : Container )
	  if( s == Value )
		return true;
	
	return false;
}

// -----------------------------------------------------------------------------

bool RandomBoolean()
{
    return (rand() % 2 == 1);
}

// -----------------------------------------------------------------------------

int RandomIntBetween( int Minimum, int Maximum )
{
    return Minimum + rand() % (Maximum - Minimum + 1);
}

// -----------------------------------------------------------------------------

float RandomFloatBetween( float Minimum, float Maximum, float Increment )
{
    int MaxIncrements = (Maximum - Minimum) / Increment;
    return Minimum + Increment * RandomIntBetween( 0, MaxIncrements );
}
