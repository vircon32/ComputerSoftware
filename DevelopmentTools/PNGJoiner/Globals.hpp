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


// program configuration parameters
extern bool VerboseMode;
extern int GapBetweenImages;
extern float HotspotProportionX;
extern float HotspotProportionY;

// working objects
extern std::list< PNGImage > LoadedImages;
extern std::list< PNGImage* > SortedImages;
extern RectangleNode TextureRectangle;


// =============================================================================
//      CODE GENERATION FUNCTIONS
// =============================================================================


// exports an XML project for the texture region
// editor tool, with all regions for the generated texture
void SaveRegionEditorProject( const std::string& FilePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
