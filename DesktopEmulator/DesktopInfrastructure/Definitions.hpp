// *****************************************************************************
    // start include guard
    #ifndef DEFINITIONS_HPP
    #define DEFINITIONS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
    #include <list>             // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      MATH - ANGLE RELATED FUNCTIONS
// =============================================================================


// math constants
#define PI     3.14159265358979323846
#define _2_PI  6.28318530717958647692
#define PI_2   1.57079632679489661923

// result is in degrees
double CalculateAngle( double DeltaX, double DeltaY );

// radian-degree conversions
extern const double DegreesPerRadian;


// =============================================================================
//      MATH - MISCELLANEOUS FUNCTIONS
// =============================================================================


bool IsPowerOf2( unsigned Value );
unsigned NextPowerOf2( unsigned Value );

template<typename T>
void Clamp( T& Variable, int Mininum, int Maximum )
{
    if( Variable > Maximum ) Variable = Maximum;
    if( Variable < Mininum ) Variable = Mininum;
}

template<typename T>
void Clamp( T& Variable, double Mininum, double Maximum )
{
    if( Variable > Maximum ) Variable = Maximum;
    if( Variable < Mininum ) Variable = Mininum;
}

inline bool IsBetween( double Value, double Minimum, double Maximum )
{
    return (Value >= Minimum) && (Value <= Maximum);
}

// returns -1, 0 or 1
template<typename T>
int Sign( T Number )
{
    return (Number > 0) - (Number < 0);
}

// define these because STL templates only work with same type arguments
float Min( float x, int y );
float Min( int x, float y );
float Max( float x, int y );
float Max( int x, float y );


// =============================================================================
//      MISCELLANEOUS FUNCTIONS
// =============================================================================


// swapping a pair of variables
template< typename T >
void Swap( T& Variable1, T& Variable2 )
{
    T Aux = Variable1;
    Variable1 = Variable2;
    Variable2 = Aux;
}

bool ListContains( std::list<std::string> Container, std::string Value );

// random number generation
bool RandomBoolean();
int RandomIntBetween( int Minimum, int Maximum );
float RandomFloatBetween( float Minimum, float Maximum, float Increment );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
