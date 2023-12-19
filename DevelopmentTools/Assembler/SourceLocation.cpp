// *****************************************************************************
    // include our headers
    #include "SourceLocation.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      WORKING WITH LOCATIONS
// =============================================================================


bool AreInSameLine( const SourceLocation& L1, const SourceLocation& L2 )
{
    // NOTE: comparing just the line is not enough
    // (includes can mix different source files)
    
    if( L1.FilePath != L2.FilePath )
      return false;
    
    return (L1.Line == L2.Line);
}
