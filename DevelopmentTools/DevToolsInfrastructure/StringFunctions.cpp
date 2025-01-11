// *****************************************************************************
    // include project headers
    #include "StringFunctions.hpp"
    
    // include C/C++ headers
    #include <iomanip>      // [ C++ STL ] I/O Manipulation
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <sstream>      // [ C++ STL ] String streams
    #include <algorithm>    // [ C++ STL ] Algorithms
    
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

// -----------------------------------------------------------------------------

// this replaces ALL occurences, in place
void ReplaceCharacter( string& Text, char OldChar, char NewChar )
{
    replace( Text.begin(), Text.end(), OldChar, NewChar );
}

// -----------------------------------------------------------------------------

// this replaces ALL occurences, in place
void ReplaceSubstring( string& Text, const string& OldSubstring, const string& NewSubstring )
{
    size_t Position = 0;
    
    while( (Position = Text.find( OldSubstring, Position )) != string::npos )
    {
        Text.replace( Position, OldSubstring.length(), NewSubstring );
        Position += NewSubstring.length();
    }
}


// =============================================================================
//      TREATMENT OF XML STRINGS
// =============================================================================


string XMLBlock( const string& BlockName, const string& BlockContent )
{
    return "<" + BlockName + ">" + BlockContent + "</" + BlockName + ">";
}

// -----------------------------------------------------------------------------

string EscapeXML( const string& Unescaped )
{
    string Escaped = Unescaped;
    
    // replace ampersand before anything else,
    // since other escape sequences will add
    // extra ampersands that we shouldn't escape
    ReplaceSubstring( Escaped, "&", "&amp;" );
    ReplaceSubstring( Escaped, "<", "&lt;" );
    ReplaceSubstring( Escaped, ">", "&gt;" );
    ReplaceSubstring( Escaped, "'", "&apos;" );
    ReplaceSubstring( Escaped, "\"", "&quot;" );
    return Escaped;
}


// =============================================================================
//      NUMBER FORMATTING FUNCTIONS
// =============================================================================


string Hex( uint32_t Value, int Digits )
{
    std::stringstream ss;
    ss << nouppercase << "0x";
    ss << uppercase << hex << setfill('0') << setw(Digits);
    ss << Value;

    return ss.str();
}

// -----------------------------------------------------------------------------

string Bin( uint32_t Value, int Digits )
{
    std::stringstream ss;
    
    for( int Bit = Digits-1; Bit >= 0; Bit-- )
    {
        unsigned Mask = 1 << Bit;
        bool HasBit = (Value & Mask);
        ss << (HasBit? '1' : '0');
    }
    
    return ss.str();
}
