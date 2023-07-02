// *****************************************************************************
    // start include guard
    #ifndef DEFINITIONS_HPP
    #define DEFINITIONS_HPP
// *****************************************************************************


// =============================================================================
//      MATH - MISCELLANEOUS FUNCTIONS
// =============================================================================


template<typename T>
void Clamp( T& Variable, int Mininum, int Maximum )
{
    if( Variable > Maximum ) Variable = Maximum;
    if( Variable < Mininum ) Variable = Mininum;
}

// -----------------------------------------------------------------------------

template<typename T>
void Clamp( T& Variable, double Mininum, double Maximum )
{
    if( Variable > Maximum ) Variable = Maximum;
    if( Variable < Mininum ) Variable = Mininum;
}

// -----------------------------------------------------------------------------

inline bool IsBetween( double Value, double Minimum, double Maximum )
{
    return (Value >= Minimum) && (Value <= Maximum);
}

// -----------------------------------------------------------------------------

inline unsigned NextPowerOf2( unsigned Value )
{
    unsigned PowerOf2 = 1;
    
    while( PowerOf2 < Value )
      PowerOf2 <<= 1;
    
    return PowerOf2;
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
