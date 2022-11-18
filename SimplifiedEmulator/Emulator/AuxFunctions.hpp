// *****************************************************************************
    // start include guard
    #ifndef AUXFUNCTIONS_HPP
    #define AUXFUNCTIONS_HPP
// *****************************************************************************


// =============================================================================
//      GENERAL USE AUXILIARY FUNCTIONS
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


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
