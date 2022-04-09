// *****************************************************************************
    // include project headers
    #include "MemoryPlacement.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      MEMORY PLACEMENT: CLASS
// =============================================================================


MemoryPlacement::MemoryPlacement()
{
    // global info
    IsGlobal = false;
    GlobalAddress = 0;
    UsesGlobalName = false;
    
    // local info
    OffsetFromBP = 0;
    
    // embedded info
    IsEmbedded = false;
}

// -----------------------------------------------------------------------------

// cannot be used for an embedded variable!
// (since its address is not considered static)
void MemoryPlacement::AddOffset( int Offset )
{
    if( Offset == 0 )
      return;
    
    if( IsGlobal )
    {
        GlobalAddress += Offset;
        UsesGlobalName = false;
    }
    
    else
      OffsetFromBP += Offset;
}

// -----------------------------------------------------------------------------

// valid for any variable type
string MemoryPlacement::AccessAddressString()
{
    // embedded placement
    if( IsEmbedded )
      return "__embedded_" + EmbeddedName;
    
    // global placement
    if( IsGlobal )
    {
        if( UsesGlobalName )
          return "global_" + GlobalName;
        
        else
          return to_string( GlobalAddress );
    }
    
    // local placement
    string OffsetString;
    
    // * local stack frame starts on [BP-1]
    // * passed parameters start on [BP+2]
    // Because of this we will always have
    // a sign initially. However we can
    // operate with offsets, so 0 is possible
    if( OffsetFromBP > 0 )
      OffsetString = "+" + to_string( OffsetFromBP );
    
    if( OffsetFromBP < 0 )
      OffsetString = to_string( OffsetFromBP );
    
    return "BP" + OffsetString;
}

// -----------------------------------------------------------------------------

// only valid for an argument!
string MemoryPlacement::PassingAddressString()
{
    // passed arguments begin at [SP] and grow positively
    int OffsetFromSP = OffsetFromBP - 2;
    
    // simplified version for no offset
    if( OffsetFromSP == 0 )
      return "SP";
    
    // otherwise build string with offset
    // (sign cannot be negative here)
    string PassingAddress = "SP+";
    PassingAddress += to_string( OffsetFromSP );
    
    return PassingAddress;
}
