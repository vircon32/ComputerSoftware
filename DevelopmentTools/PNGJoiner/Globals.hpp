// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include project headers
    #include "PNGImage.hpp"
    #include "RectangleNode.hpp"
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <list>         // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


extern bool VerboseMode;
extern int GapBetweenImages;
extern std::list< PNGImage > LoadedImages;
extern RectangleNode TextureRectangle;


// =============================================================================
//      CODE GENERATION FUNCTIONS
// =============================================================================


void SaveCTextureHeader( const std::string& HeaderPath );
void SaveCTextureSource( const std::string& SourcePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
