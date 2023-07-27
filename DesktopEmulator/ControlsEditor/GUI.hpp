// *****************************************************************************
    // start include guard
    #ifndef GUI_HPP
    #define GUI_HPP
    
    // include C/C++ headers
    #include <string>                  // [ C++ STL ] Strings
    
    // include imgui headers
    #include <imgui/imgui.h>                // [ Dear ImGui ] Main header
    #include <imgui/imgui_impl_sdl.h>       // [ Dear ImGui ] SDL2 backend header
    #include <imgui/imgui_impl_opengl3.h>   // [ Dear ImGui ] OpenGL 3 backend header
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
