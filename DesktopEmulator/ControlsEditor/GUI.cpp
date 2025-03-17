// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/StringFunctions.hpp"
    
    // include controls editor headers
    #include "OpenGL2DContext.hpp"
    #include "GUI.hpp"
    #include "Controls.hpp"
    #include "Globals.hpp"
    #include "Languages.hpp"
    
    // include C/C++ headers
    #include <set>                  // [ C++ STL ] Sets
    
    // include osdialog headers
    #include <osdialog/osdialog.h>  // [ osdialog ] Main header
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      TEXT ALIGNMENT
// =============================================================================


void TextAlignedLeft( const string& Text )
{
    ImGui::Text( Text.c_str() );
}

// -----------------------------------------------------------------------------

void TextAlignedCenter( const string& Text )
{
    ImVec2 ScreenPosition = ImGui::GetCursorScreenPos();
    float TextWidth   = ImGui::CalcTextSize( Text.c_str() ).x;
    ScreenPosition.x -= TextWidth / 2;
    ImGui::SetCursorScreenPos(ScreenPosition);

    ImGui::Text( Text.c_str() );
}

// -----------------------------------------------------------------------------

void TextAlignedRight( const string& Text )
{
    ImVec2 ScreenPosition = ImGui::GetCursorScreenPos();
    float TextWidth   = ImGui::CalcTextSize( Text.c_str() ).x;
    ScreenPosition.x -= TextWidth;
    ImGui::SetCursorScreenPos(ScreenPosition);

    ImGui::Text( Text.c_str() );
}


// =============================================================================
//      GUI FUNCTIONS FOR KEYBOARD PROFILE
// =============================================================================


string WriteKeyCode( SDL_Keycode Key )
{
    // CASE 1: key
    if( Key >= 0 )
      return string("Key ") + SDL_GetKeyName( Key );
    
    // CASE 2: unmapped
    return "";
}

// -----------------------------------------------------------------------------

// it is needed to specify a button ID text. ImGui uses that internally
// as button identification, and without it we risk that buttons do not
// work if their labels are empty or repeated in more than 1 button
void ProcessKeyButton( SDL_Keycode* TargetKey, const string& ButtonID )
{
    // notation "aa##bb" in text label is used in ImGui to specify
    // both a label text "aa" and a button identification "bb"
    string ButtonLabel = WriteKeyCode( *TargetKey ) + "##" + ButtonID;
    
    if( ImGui::Button( ButtonLabel.c_str(), ImVec2(100,25) ) )
    {
        ImGui::OpenPopup( Texts(TextIDs::WaitKeyboard_Waiting) );
        KeyBeingMapped = TargetKey;
    }
}

// -----------------------------------------------------------------------------

void ProcessWaitForKeyboardWindow()
{
    if( ImGui::BeginPopupModal( Texts(TextIDs::WaitKeyboard_Waiting) ) )
    {
        ImGui::Text( Texts(TextIDs::WaitKeyboard_PressKey) );
        ImGui::Text( Texts(TextIDs::WaitKeyboard_EscapeCancel) );
        ImGui::Text(" ");
        
        if( ImGui::Button( Texts(TextIDs::WaitKeyboard_Clear) ) )
        {
            ImGui::CloseCurrentPopup();
            *KeyBeingMapped = -1;
            KeyBeingMapped = nullptr;
        }
        
        ImGui::SameLine();
        
        if( ImGui::Button( Texts(TextIDs::Dialogs_ButtonCancel) ) || !KeyBeingMapped )
        {
            ImGui::CloseCurrentPopup();
            KeyBeingMapped = nullptr;
        }
        
        ImGui::EndPopup();
    }
}

// -----------------------------------------------------------------------------

void DisplayKeyboardMapping()
{
    // show profile name (NOT editable)
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(0,0,1,1.0) );
    ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1.0 );
    {
        ImGui::SetCursorScreenPos(ImVec2(216,47));
        ImGui::SetNextItemWidth( 345 );
        
        strcpy( ProfileName, Texts(TextIDs::Info_Keyboard) );
        ImGui::InputText( "", ProfileName, 40, ImGuiInputTextFlags_ReadOnly );
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    // write device name and GUID
    ImGui::SetCursorScreenPos(ImVec2(220,70));
    ImGui::Text( Texts(TextIDs::Info_Keyboard) );
    
    ImGui::SetCursorScreenPos(ImVec2(220,90));
    ImGui::Text( Texts(TextIDs::Info_NoGUID) );
    
    // write left mappings
    ImGui::SetCursorScreenPos(ImVec2(19,164));
    ProcessKeyButton( &KeyboardProfile.ButtonL, "ButtonL" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29));
    ProcessKeyButton( &KeyboardProfile.Up, "Up" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*2));
    ProcessKeyButton( &KeyboardProfile.Left, "Left" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*3));
    ProcessKeyButton( &KeyboardProfile.Down, "Down" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*4));
    ProcessKeyButton( &KeyboardProfile.Right, "Right" );
    
    // write right mappings
    ImGui::SetCursorScreenPos(ImVec2(369,164));
    ProcessKeyButton( &KeyboardProfile.ButtonR, "ButtonR" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29));
    ProcessKeyButton( &KeyboardProfile.ButtonX, "ButtonX" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*2));
    ProcessKeyButton( &KeyboardProfile.ButtonA, "ButtonA" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*3));
    ProcessKeyButton( &KeyboardProfile.ButtonB, "ButtonB" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*4));
    ProcessKeyButton( &KeyboardProfile.ButtonY, "ButtonY" );
    
    // write start mapping
    ImGui::SetCursorScreenPos(ImVec2(192,298));
    ProcessKeyButton( &KeyboardProfile.ButtonStart, "ButtonStart" );
    
    // write command mapping
    ImGui::SetCursorScreenPos(ImVec2(508,164+29*3));
    ProcessKeyButton( &KeyboardProfile.Command, "Command" );
    
    // modal window to change a key
    ProcessWaitForKeyboardWindow();
}


// =============================================================================
//      GUI FUNCTIONS FOR JOYSTICK PROFILES
// =============================================================================


string WriteJoystickControl( JoystickControl C )
{
    // CASE 1: axis
    if( C.IsAxis() )
      return "Axis " + to_string( C.AxisIndex ) + (C.AxisPositive? " plus" : " minus");
    
    // CASE 2: hat
    if( C.IsHat() )
    {
        string DirectionText = "";
        
        if( C.HatDirection & SDL_HAT_LEFT )
          DirectionText = "left";
        
        else if( C.HatDirection & SDL_HAT_RIGHT )
          DirectionText = "right";
        
        else if( C.HatDirection & SDL_HAT_UP )
          DirectionText = "up";
        
        else if( C.HatDirection & SDL_HAT_DOWN )
          DirectionText = "down";
          
        return "Hat " + to_string( C.HatIndex ) + " " + DirectionText;
    }
    
    // CASE 3: button
    if( C.IsButton() )
      return "Button " + to_string( C.ButtonIndex );
    
    // CASE 4: unmapped
    return "";
}

// -----------------------------------------------------------------------------

// it is needed to specify a button ID text. ImGui uses that internally
// as button identification, and without it we risk that buttons do not
// work if their labels are empty or repeated in more than 1 button
void ProcessJoystickButton( JoystickControl* TargetControl, const string& ButtonID )
{
    // notation "aa##bb" in text label is used in ImGui to specify
    // both a label text "aa" and a button identification "bb"
    string ButtonLabel = WriteJoystickControl( *TargetControl ) + "##" + ButtonID;
    
    if( ImGui::Button( ButtonLabel.c_str(), ImVec2(100,25) ) )
    {
        ImGui::OpenPopup( Texts(TextIDs::WaitJoystick_Waiting) );
        ControlBeingMapped = TargetControl;
    }
}

// -----------------------------------------------------------------------------

void ProcessWaitForJoystickWindow()
{
    if( ImGui::BeginPopupModal( Texts(TextIDs::WaitJoystick_Waiting) ) )
    {
        ImGui::Text( Texts(TextIDs::WaitJoystick_PressControl) );
        ImGui::Text( Texts(TextIDs::WaitJoystick_EscapeCancel) );
        ImGui::Text(" ");
        
        if( ImGui::Button( Texts(TextIDs::WaitJoystick_Clear) ) )
        {
            ImGui::CloseCurrentPopup();
            ControlBeingMapped->Type = JoystickControlTypes::None;
            ControlBeingMapped->ButtonIndex = -1;
            ControlBeingMapped->AxisIndex = -1;
            ControlBeingMapped->HatIndex = -1;
            ControlBeingMapped = nullptr;
        }
        
        ImGui::SameLine();
        
        if( ImGui::Button( Texts(TextIDs::Dialogs_ButtonCancel) ) || !ControlBeingMapped )
        {
            ImGui::CloseCurrentPopup();
            ControlBeingMapped = nullptr;
        }
        
        ImGui::EndPopup();
    }
}

// -----------------------------------------------------------------------------

void DisplayJoystickMapping( JoystickMapping* SelectedProfile )
{
    // show profile name (editable)
    ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4(0.75,1,1,1.0) );
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(0,0,1,1.0) );
    ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1.0 );
    {
        ImGui::SetCursorScreenPos(ImVec2(216,47));
        ImGui::SetNextItemWidth( 345 );
        
        strncpy( ProfileName, SelectedProfile->ProfileName.c_str(), 40 );
        
        if( ImGui::InputText( "", ProfileName, 40, ImGuiInputTextFlags_None ) )
          SelectedProfile->ProfileName = ProfileName;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    
    // write device name and GUID
    ImGui::SetCursorScreenPos(ImVec2(220,70));
    ImGui::Text( SelectedProfile->JoystickName.c_str() );
    
    ImGui::SetCursorScreenPos(ImVec2(220,90));
    char GUIDString[ 40 ];
    SDL_JoystickGetGUIDString( SelectedProfile->GUID, GUIDString, 39 );
    ImGui::Text( GUIDString );
    
    // write left mappings
    ImGui::SetCursorScreenPos(ImVec2(19,164));
    ProcessJoystickButton( &SelectedProfile->ButtonL, "ButtonL" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29));
    ProcessJoystickButton( &SelectedProfile->Up, "Up" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*2));
    ProcessJoystickButton( &SelectedProfile->Left, "Left" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*3));
    ProcessJoystickButton( &SelectedProfile->Down, "Down" );
    
    ImGui::SetCursorScreenPos(ImVec2(19,164+29*4));
    ProcessJoystickButton( &SelectedProfile->Right, "Right" );
    
    // write right mappings
    ImGui::SetCursorScreenPos(ImVec2(369,164));
    ProcessJoystickButton( &SelectedProfile->ButtonR, "ButtonR" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29));
    ProcessJoystickButton( &SelectedProfile->ButtonX, "ButtonX" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*2));
    ProcessJoystickButton( &SelectedProfile->ButtonA, "ButtonA" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*3));
    ProcessJoystickButton( &SelectedProfile->ButtonB, "ButtonB" );
    
    ImGui::SetCursorScreenPos(ImVec2(369,164+29*4));
    ProcessJoystickButton( &SelectedProfile->ButtonY, "ButtonY" );
    
    // write start mapping
    ImGui::SetCursorScreenPos(ImVec2(192,298));
    ProcessJoystickButton( &SelectedProfile->ButtonStart, "ButtonStart" );
    
    // write command mapping
    ImGui::SetCursorScreenPos(ImVec2(508,164+29*3));
    ProcessJoystickButton( &SelectedProfile->Command, "Command" );
    
    // modal window to change a key
    ProcessWaitForJoystickWindow();
}


// =============================================================================
//      WRAPPERS TO LOAD AND SAVE FILES
// =============================================================================


string GetLoadFilePath( const char* Filters )
{
    // show the dialog with the requested filter
    osdialog_filters* ParsedFilters = osdialog_filters_parse( Filters );
    char* FilePath = osdialog_file( OSDIALOG_OPEN, ProgramFolder.c_str(), "", ParsedFilters );
    osdialog_filters_free( ParsedFilters );
    
    // when cancelled, return empty string
    if( !FilePath )
      return "";
    
    // we need to save the result in a string
    string Result = FilePath;
    free( FilePath );
    return Result;
}

// -----------------------------------------------------------------------------

string GetSaveFilePath( const char* Filters )
{
    // show the dialog with the requested filter
    osdialog_filters* ParsedFilters = osdialog_filters_parse( Filters );
    char* FilePath = osdialog_file( OSDIALOG_SAVE, ProgramFolder.c_str(), "", ParsedFilters );
    osdialog_filters_free( ParsedFilters );
    
    // when cancelled, return empty string
    if( !FilePath )
      return "";
    
    // we need to save the result in a string
    string Result = FilePath;
    free( FilePath );
    return Result;
}


// =============================================================================
//      INDIVIDUAL MENUS IN THE MENU BAR
// =============================================================================


bool ShowMessageBoxYesNo( const string& Title, const string& Message )
{
    SDL_MessageBoxButtonData MessageBoxButtons[] =
    {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, Texts(TextIDs::Dialogs_ButtonNo) },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, Texts(TextIDs::Dialogs_ButtonYes) }
    };
    
    SDL_MessageBoxData MessageBoxData =
    {
        SDL_MESSAGEBOX_WARNING,             // flags
        nullptr,                            // window
        Title.c_str(),                      // title
        Message.c_str(),                    // message
        SDL_arraysize( MessageBoxButtons ), // numbuttons
        MessageBoxButtons,                  // buttons
        nullptr                             // colorScheme
    };
    
    int PressedButton = 0;
    SDL_ShowMessageBox( &MessageBoxData, &PressedButton );
    
    return PressedButton;
}

// -----------------------------------------------------------------------------

void ProcessMenuFile()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_File) ) )
      return;
    
    if( ImGui::MenuItem( Texts(TextIDs::File_SetDefaults) ) )
      if( ShowMessageBoxYesNo( Texts(TextIDs::Dialogs_AreYouSure), Texts(TextIDs::Dialogs_SetDefaults_Label) ) )
        SetDefaultControls();
       
    if( ImGui::MenuItem( Texts(TextIDs::File_ReloadControls) ) )
      if( ShowMessageBoxYesNo( Texts(TextIDs::Dialogs_AreYouSure), Texts(TextIDs::Dialogs_Reload_Label) ) )
        LoadControls( ProgramFolder + "Config-Controls.xml" );
       
    if( ImGui::MenuItem( Texts(TextIDs::File_ExitSave) ) )
      if( ShowMessageBoxYesNo( Texts(TextIDs::Dialogs_AreYouSure), Texts(TextIDs::Dialogs_ExitSave_Label) ) )
      {
          SaveControls( ProgramFolder + "Config-Controls.xml" );
          GlobalLoopActive = false;
      }
    
    if( ImGui::MenuItem( Texts(TextIDs::File_ExitNoSave) ) )
      if( ShowMessageBoxYesNo( Texts(TextIDs::Dialogs_AreYouSure), Texts(TextIDs::Dialogs_ExitNoSave_Label) ) )
         GlobalLoopActive = false;
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuDevices()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Devices) ) )
      return;
    
    if( ImGui::BeginMenu( Texts(TextIDs::Devices_Keyboard) ) )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Devices_EditProfile) ) )
          SelectedProfile = nullptr;
        
        ImGui::EndMenu();
    }
    
    // we will show each GUID only once
    set< SDL_JoystickGUID > ShownGUIDs;
    
    for( auto Pair: ConnectedJoysticks )
    {
        // get joystick name
        SDL_JoystickGUID GUID = Pair.second;
        
        if( ShownGUIDs.count( GUID ) > 0 )
          continue;
        
        ShownGUIDs.insert( GUID );
        
        SDL_Joystick* Joystick = SDL_JoystickFromInstanceID( Pair.first );
        string JoystickName = SDL_JoystickName( Joystick );
        
        // show submenu option
        if( ImGui::BeginMenu( JoystickName.c_str() ) )
        {
            // check if joystick has a profile
            auto ProfilesPair = JoystickProfiles.find( GUID );
            bool ProfileExists = (ProfilesPair != JoystickProfiles.end());
            
            if( ProfileExists )
            {
                if( ImGui::MenuItem( Texts(TextIDs::Devices_EditProfile) ) )
                  SelectedProfile = ProfilesPair->second;
                  
                if( ImGui::MenuItem( Texts(TextIDs::Devices_DeleteProfile) ) )
                {
                    delete ProfilesPair->second;
                    JoystickProfiles.erase( ProfilesPair );
                    
                    // update selected profile if needed
                    if( SelectedProfile == ProfilesPair->second )
                      SelectedProfile = nullptr;
                }
            }
            
            else
            {
                if( ImGui::MenuItem( Texts(TextIDs::Devices_CreateProfile) ) )
                {
                    // create new profile
                    JoystickMapping* NewProfile = new JoystickMapping;
                    NewProfile->ProfileName = JoystickName;
                    NewProfile->JoystickName = JoystickName;
                    NewProfile->GUID = GUID;
                    
                    // add it to the list
                    JoystickProfiles[ GUID ] = NewProfile;
                    
                    // select it
                    SelectedProfile = NewProfile;
                };
            }
            ImGui::EndMenu();
        }
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuProfiles()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Profiles) ) )
      return;
    
    if( ImGui::BeginMenu( Texts(TextIDs::Profiles_Keyboard) ) )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Profiles_Edit) ) )
          SelectedProfile = nullptr;
        
        ImGui::EndMenu();
    }
    
    for( auto Position = JoystickProfiles.cbegin(); Position != JoystickProfiles.cend(); Position++ )
    {
        // get profile
        JoystickMapping* JoystickProfile = Position->second;
        
        // show submenu option
        if( ImGui::BeginMenu( JoystickProfile->ProfileName.c_str() ) )
        {
            if( ImGui::MenuItem( Texts(TextIDs::Profiles_Edit) ) )
              SelectedProfile = JoystickProfile;
              
            if( ImGui::MenuItem( Texts(TextIDs::Profiles_Delete) ) )
            {
                delete JoystickProfile;
                JoystickProfiles.erase( Position );
                    
                // update selected profile if needed
                if( SelectedProfile == JoystickProfile )
                   SelectedProfile = nullptr;
            }
              
            ImGui::EndMenu();
        }
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuLanguage()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Language) ) )
      return;
    
    if( ImGui::MenuItem( Texts(TextIDs::Language_English), nullptr, (CurrentLanguage == &LanguageEnglish[0]), true ) )
      SetLanguage( "English" );
          
    if( ImGui::MenuItem( Texts(TextIDs::Language_Spanish), nullptr, (CurrentLanguage == &LanguageSpanish[0]), true ) )
      SetLanguage( "Spanish" );

    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuHelp()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Help) ) )
      return;
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_QuickGuide) ) )
    {
        SDL_ShowSimpleMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts(TextIDs::Dialogs_Guide_Title),
            Texts(TextIDs::Dialogs_Guide_Text),
            nullptr
        );
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_About) ) )
    {
        SDL_ShowSimpleMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts(TextIDs::Dialogs_About_Title),
            Texts(TextIDs::Dialogs_About_Text),
            nullptr
        );
    }
       
    ImGui::EndMenu();
}


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


void RenderGUI()
{
    // start new frame in imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame( OpenGL2D.Window );
    ImGui::NewFrame();
    
    // draw background
    ImGuiWindowFlags WindowFlags =
    (
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse
    );
    
    ImGui::SetNextWindowPos(ImVec2(0,20));
    ImGui::SetNextWindowSize(ImVec2(640,339));
    
    ImGui::Begin("Window", nullptr, WindowFlags);
    {
        ImGui::SetCursorScreenPos(ImVec2(100,50));
        ImGui::Text( Texts(TextIDs::Info_ProfileName) );
        ImGui::SetCursorScreenPos(ImVec2(100,70));
        ImGui::Text( Texts(TextIDs::Info_DeviceName) );
        ImGui::SetCursorScreenPos(ImVec2(100,90));
        ImGui::Text( Texts(TextIDs::Info_DeviceGUID) );
        
        ImGui::SetCursorScreenPos(ImVec2(0,130));
        ImGui::Separator();
        
        ImGui::SetCursorScreenPos(ImVec2(19,163));
        ImGui::Image( (void*)GamepadTextureID, ImVec2(1024,256) );
        
        // now write the actual values
        if( SelectedProfile )
          DisplayJoystickMapping( SelectedProfile );  
        else
          DisplayKeyboardMapping();
        
        // draw texts for command button
        ImGui::SetCursorScreenPos(ImVec2(557,168));
        TextAlignedCenter( Texts(TextIDs::Command_Name) );
        ImGui::SetCursorScreenPos(ImVec2(557,288));
        TextAlignedCenter( Texts(TextIDs::Command_TextLine1) );
        ImGui::SetCursorScreenPos(ImVec2(557,304));
        TextAlignedCenter( Texts(TextIDs::Command_TextLine2) );
    }
    ImGui::End();
    
    // show the main menu bar
    if( ImGui::BeginMainMenuBar() )
    {
        // menus
        ProcessMenuFile();
        ProcessMenuDevices();
        ProcessMenuProfiles();
        ProcessMenuLanguage();
        ProcessMenuHelp();
        
        ImGui::EndMainMenuBar();
    }
    
    // end ImGui frame
    ImGui::EndFrame();
    
    // render imgui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}
