// *****************************************************************************
    // start include guard
    #ifndef GUI_HPP
    #define GUI_HPP
    
    // include imgui headers
    #include <imgui/imgui.h>
    #include <imgui/imgui_impl_sdl.h>
    #include <imgui/imgui_impl_opengl2.h>
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      WRAPPERS TO LOAD AND SAVE FILES
// =============================================================================


std::string GetLoadFilePath( const char* Filters, const std::string& Directory );
std::string GetSaveFilePath( const char* Filters, const std::string& Directory );


// =============================================================================
//      DELAYED MESSAGE BOX FUNCTIONS
// =============================================================================


void DelayedMessageBox( uint32_t flags, const char *Title, const char *Message );
void ShowDelayedMessageBox();


// =============================================================================
//      ADDITIONAL GUI FUNCTIONS
// =============================================================================


void SetWindowZoom( int ZoomFactor );
void SetFullScreen();
void SaveScreenshot( const std::string& FilePath );
void AddRecentCartridgePath( const std::string& CartridgePath );
void AddRecentMemoryCardPath( const std::string& MemoryCardPath );
void CheckCartridgePaths();
void CheckMemoryCardPaths();


// =============================================================================
//      ENCAPSULATED GUI FUNCTIONS
// =============================================================================


void GUI_CreateMemoryCard();
void GUI_UnloadMemoryCard();
void GUI_LoadMemoryCard( std::string MemoryCardPath = "" );
void GUI_ChangeMemoryCard( std::string MemoryCardPath = "" );
void GUI_UnloadCartridge();
void GUI_LoadCartridge( std::string CartridgePath = "" );
void GUI_ChangeCartridge( std::string CartridgePath = "" );
void GUI_SaveScreenshot( std::string FilePath = "" );


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


bool GUIMustBeDrawn();
void RenderGUI();
void ShowEmulatorWindow();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
