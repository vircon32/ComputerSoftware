// *****************************************************************************
    // include project headers
    #include "VirconROMFormat.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO WORK WITH SIGNATURES
// =============================================================================


void WriteSignature( ofstream& OutputFile, const char* Value )
{
    OutputFile.write( Value, 8 );
}

// -----------------------------------------------------------------------------

bool CheckSignature( char* Signature, const char* Expected )
{
    for( int i = 0; i < 8; i++ )
      if( Signature[i] != Expected[i] )
        return false;
    
    return true;
}
