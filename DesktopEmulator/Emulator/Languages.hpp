// *****************************************************************************
    // start include guard
    #ifndef LANGUAGES_HPP
    #define LANGUAGES_HPP
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <map>          // [ C++ STL ] Maps
// *****************************************************************************


// enumerates all texts present in the program
enum class TextIDs: int
{
    None = 0,
    Menus_Console,
    Menus_Cartridge,
    Menus_Card,
    Menus_Gamepads,
    Menus_Options,
    Menus_Help,
    Console_PowerOn,
    Console_PowerOff,
    Console_Reset,
    Console_Quit,
    Cartridge_NoCartridge,
    Cartridge_Locked,
    Cartridge_Load,
    Cartridge_Unload,
    Cartridge_Change,
    Cartridge_RecentTitle,
    Cartridge_RecentEmpty,
    Cartridge_RecentClear,
    Card_NoCard,
    Card_Create,
    Card_Load,
    Card_Unload,
    Card_Change,
    Card_RecentTitle,
    Card_RecentEmpty,
    Card_RecentClear,
    Gamepads_Gamepad,
    Gamepads_NoDevice,
    Gamepads_Keyboard,
    Options_VideoSize,
    Options_SoundVolume,
    Options_MemoryCards,
    Options_Language,
    Options_Screenshot,
    Options_FullScreen,
    Options_Mute,
    Options_CardsAuto,
    Options_CardsManual,
    Options_English,
    Options_Spanish,
    Help_QuickGuide,
    Help_ShowReadme,
    Help_About,
    Status_ConsoleOff,
    Status_CPUHalted,
    Status_NoCartridge,
    
    Dialogs_ButtonOK,
    Dialogs_ButtonCancel,
    Dialogs_ButtonYes,
    Dialogs_ButtonNo,
    Dialogs_Done,
    Dialogs_CardCreated_Label,
    Dialogs_ScreenshotSaved_Label,
    Dialogs_About_Title,
    Dialogs_About_Label,
    Dialogs_Guide_Title,
    Dialogs_Guide_Label,
    FileNames_Readme,
    
    Errors_TopLevel_Title,
    Errors_TopLevel_Label,
    Errors_CreateCard_Label,
    Errors_LoadCard_Label,
    Errors_UnloadCard_Label,
    Errors_ChangeCard_Label,
    Errors_AutoUpdateCard_Label,
    Errors_LoadCartridge_Label,
    Errors_UnloadCartridge_Label,
    Errors_ChangeCartridge_Label,
    Errors_SaveScreenshot_Label,
    Errors_LoadControls_Label,
    Errors_LoadControls_SetDefaults,
    Errors_InvalidDevice_Title,
    Errors_InvalidDevice_CannotFind,
    Errors_InvalidDevice_Gamepad,
    Errors_InvalidDevice_SetNoDevice,
    Errors_LoadSettings_Label,
    Errors_LoadSettings_SetDefaults,
    Errors_SaveSettings_Label,
    Errors_InvalidLanguage_Title,
    Errors_InvalidLanguage_Label
};

// -----------------------------------------------------------------------------

extern std::string LanguageEnglish[];
extern std::string LanguageSpanish[];
extern std::map< std::string, std::string* > Languages;
extern std::string* CurrentLanguage;

// -----------------------------------------------------------------------------

void SetLanguage( std::string LanguageName );
const char* Texts( TextIDs ID );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
