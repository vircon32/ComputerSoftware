// *****************************************************************************
    // start include guard
    #ifndef SETTINGS_HPP
    #define SETTINGS_HPP
    
    // include common Vircon headers
    #include "../VirconDefinitions/Constants.hpp"
    
    // include C/C++ headers
    #include <string>               // [ C++ STL ] Strings
    #include <map>                  // [ C++ STL ] Maps
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include <SDL2/SDL.h>           // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      LOAD INPUT DEVICES FROM XML FILE
// =============================================================================


void LoadControls( const std::string& FilePath );


// =============================================================================
//      LOAD & SAVE SETTINGS FROM XML FILES
// =============================================================================


void SetDefaultSettings();
void LoadSettings( const std::string& FilePath );
void SaveSettings( const std::string& FilePath );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

