// *****************************************************************************
    // start include guard
    #ifndef DEFINITIONS_HPP
    #define DEFINITIONS_HPP
// *****************************************************************************


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


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
