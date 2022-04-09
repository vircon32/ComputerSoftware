// *****************************************************************************
    // include project headers
    #include "RegisterAllocation.hpp"
    #include "CompilerInfrastructure.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: REGISTER ALLOCATION
// =============================================================================


RegisterAllocation::RegisterAllocation( SourceLocation Location_ )
{
    Location = Location_;
    
    // initially nothing is used
    for( bool& R: RegisterUsed )
      R = false;
    
    TemporariesStackSize = 0;
    HighestUsedRegister = 0;
}

// -----------------------------------------------------------------------------

int RegisterAllocation::FirstFreeRegister()
{
    // Register R0 should never be used for calculations,
    // only for function calls or to return a value.
    // Registers BP and SP (indices 14 and 15) are used
    // to control the stack so they are not used either
    for( int i = 1; i < 14; i++ )
    if( !RegisterUsed[ i ] )
    {
        // track the highest used register
        if( i > HighestUsedRegister )
          HighestUsedRegister = i;
        
        // mark the register as used
        RegisterUsed[ i ] = true;
        return i;
    }
    
    // no free register was found
    RaiseFatalError( Location, "expression is too complex, try splitting it into simpler expressions" );
}
