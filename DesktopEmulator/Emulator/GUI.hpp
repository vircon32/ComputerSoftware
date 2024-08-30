// *****************************************************************************
    // start include guard
    #ifndef GUI_HPP
    #define GUI_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <cstdint>          // [ ANSI C ] Standard integer types
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
std::string GetAutomaticMemoryCardPath( const std::string& CartridgePath );
std::string GetAutomaticSaveStatePath( const std::string& CartridgePath );


// =============================================================================
//      ENCAPSULATED GUI FUNCTIONS
// =============================================================================


void GUI_CreateMemoryCard( std::string MemoryCardPath = "" );
void GUI_UnloadMemoryCard();
void GUI_LoadMemoryCard( std::string MemoryCardPath = "" );
void GUI_ChangeMemoryCard( std::string MemoryCardPath = "" );
void GUI_AutoUpdateMemoryCard();
void GUI_UnloadCartridge();
void GUI_LoadCartridge( std::string CartridgePath = "" );
void GUI_ChangeCartridge( std::string CartridgePath = "" );
void GUI_SaveScreenshot( std::string FilePath = "" );
void GUI_LoadState();
void GUI_SaveState();


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
