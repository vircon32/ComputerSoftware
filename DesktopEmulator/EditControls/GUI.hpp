// *****************************************************************************
    // start include guard
    #ifndef GUI_HPP
    #define GUI_HPP
    
    // include imgui headers
    #include <imgui/imgui.h>
    #include <imgui/imgui_impl_sdl.h>
    #include <imgui/imgui_impl_opengl2.h>
    
    // include C++ headers
    #include <string>                  // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      GUI SUPPORT FUNCTIONS
// =============================================================================


bool ShowMessageBoxYesNo( const std::string& Title, const std::string& Message );


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


void RenderScene();
void RenderGUI();
void ShowEmulatorWindow();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
