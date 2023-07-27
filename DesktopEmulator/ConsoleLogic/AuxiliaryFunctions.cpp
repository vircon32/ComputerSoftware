// *****************************************************************************
    // include console logic headers
    #include "AuxiliaryFunctions.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      FILE HANDLING FUNCTIONS
    // =============================================================================
    
    
    // this is dependent on the host operating system
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
        char PathSeparator = '\\';
    #else
        char PathSeparator = '/';
    #endif
    
    // -----------------------------------------------------------------------------
    
    string GetPathFileName( const string& FilePath )
    {
        size_t SlashPosition = FilePath.rfind( PathSeparator );
        
        if( SlashPosition == string::npos )
          return "";
        
        if( FilePath.size() < (SlashPosition + 2) )
          return "";
        
        return FilePath.substr( SlashPosition + 1, FilePath.size() - 1 );
    }
    
    
    // =============================================================================
    //      SIGNATURE HANDLING FUNCTIONS
    // =============================================================================
    
    
    void WriteSignature( ostream& OutputFile, const char* Value )
    {
        OutputFile.write( Value, 8 );
    }
    
    // -----------------------------------------------------------------------------
    
    bool CheckSignature( char* Signature, const char* Expected )
    {
        for( int i = 0; i < 8; i++ )
          if( Signature[ i ] != Expected[ i ] )
            return false;
        
        return true;
    }
}
