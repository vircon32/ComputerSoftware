// *****************************************************************************
    // start include guard
    #ifndef SOURCELOCATION_HPP
    #define SOURCELOCATION_HPP
    
    // include external headers
    #include <string>       // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      LOCATIONS IN SOURCE FILES
// =============================================================================


class SourceLocation
{
    public:
        
        std::string FilePath;
        int LogicalLine;        // for lines continued with '\'
        int Line;
        int Column;
};


// =============================================================================
//      WORKING WITH LOCATIONS
// =============================================================================


bool AreInSameLine( const SourceLocation& L1, const SourceLocation& L2 );



// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
