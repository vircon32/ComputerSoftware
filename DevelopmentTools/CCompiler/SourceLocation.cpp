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
    // NOTE 1: comparing just the line is not enough
    // (#includes can mix different source files)
    
    // NOTE 2: we must compare the LOGICAL line, in
    // case a line has been continued with a '\'
    
    if( L1.FilePath != L2.FilePath )
      return false;
    
    return (L1.LogicalLine == L2.LogicalLine);
}
