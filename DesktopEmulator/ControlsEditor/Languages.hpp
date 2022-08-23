// *****************************************************************************
    // start include guard
    #ifndef LANGUAGES_HPP
    #define LANGUAGES_HPP
    
    // include external headers
    #include <string>       // [ C++ STL ] Strings
    #include <map>          // [ C++ STL ] Maps
// *****************************************************************************


// enumerates all texts present in the program
enum class TextIDs: int
{
    None = 0,
    Menus_File,
    Menus_Devices,
    Menus_Profiles,
    Menus_Language,
    Menus_Help,
    File_SetDefaults,
    File_ReloadControls,
    File_ExitSave,
    File_ExitNoSave,
    Devices_Keyboard,
    Devices_CreateProfile,
    Devices_EditProfile,
    Devices_DeleteProfile,
    Profiles_Keyboard,
    Profiles_Edit,
    Profiles_Delete,
    Language_English,
    Language_Spanish,
    Help_QuickGuide,
    Help_About,
    Info_ProfileName,
    Info_DeviceName,
    Info_DeviceGUID,
    Info_Keyboard,
    Info_NoGUID,
    
    WaitKeyboard_Waiting,
    WaitKeyboard_PressKey,
    WaitKeyboard_EscapeCancel,
    WaitKeyboard_Clear,
    WaitJoystick_Waiting,
    WaitJoystick_PressControl,
    WaitJoystick_EscapeCancel,
    WaitJoystick_Clear,
    
    Dialogs_ButtonOK,
    Dialogs_ButtonCancel,
    Dialogs_ButtonYes,
    Dialogs_ButtonNo,
    Dialogs_AreYouSure,
    Dialogs_SetDefaults_Label,
    Dialogs_Reload_Label,
    Dialogs_ExitSave_Label,
    Dialogs_ExitNoSave_Label,
    Dialogs_Delete_Label,
    Dialogs_About_Title,
    Dialogs_About_Text,
    Dialogs_Guide_Title,
    Dialogs_Guide_Text,
    
    Errors_TopLevel_Title,
    Errors_TopLevel_Label,
    Errors_LoadControls_Label,
    Errors_LoadControls_SetDefaults,
    Errors_SaveControls_Label
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
