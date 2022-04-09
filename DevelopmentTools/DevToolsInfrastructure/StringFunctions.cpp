// *****************************************************************************
    // include project headers
    #include "StringFunctions.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      STRING MANIPULATION FUNCTIONS
// =============================================================================


string ToLowerCase( const string& Text )
{
    string Result;
    
    for( char i: Text )
      Result += tolower( i );
    
    return Result;
}

// -----------------------------------------------------------------------------

string ToUpperCase( const string& Text )
{
    string Result;
    
    for( char i: Text )
      Result += toupper( i );
    
    return Result;
}

// -----------------------------------------------------------------------------

vector<string> SplitString( const string& str, char separator )
{
   vector<string> Result;
   size_t pos1 = 0;
   size_t pos2 = 0;
   
   while( pos2 != str.npos )
   {
      pos2 = str.find(separator, pos1);
      if ( pos2 != str.npos )
      {
         if ( pos2 > pos1 )
            Result.push_back( str.substr(pos1, pos2-pos1) );
         pos1 = pos2+1;
      }
   }
   
   Result.push_back( str.substr(pos1, str.size()-pos1) );
   return Result;
}
