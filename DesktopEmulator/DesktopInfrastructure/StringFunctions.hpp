// *****************************************************************************
    // start include guard
    #ifndef STRINGFUNCTIONS_HPP
    #define STRINGFUNCTIONS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


// =============================================================================
//      STRING MANIPULATION FUNCTIONS
// =============================================================================


// character-based manipulations
std::string ToLowerCase( const std::string& Text );
std::string ToUpperCase( const std::string& Text );
std::vector<std::string> SplitString( const std::string& str, char separator );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
