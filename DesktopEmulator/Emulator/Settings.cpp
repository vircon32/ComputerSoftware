// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/StringFunctions.hpp"
    
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "Settings.hpp"
    #include "GamepadsInput.hpp"
    #include "AudioOutput.hpp"
    #include "VideoOutput.hpp"
    #include "GUI.hpp"
    #include "Languages.hpp"
    #include "Globals.hpp"
    
    // include external headers
    #include <tinyxml2.h>
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
    using namespace tinyxml2;
// *****************************************************************************


// =============================================================================
//      XML HELPER FUNCTIONS
// =============================================================================


// automation for child elements in XML
XMLElement* GetRequiredElement( XMLElement* Parent, const string& ChildName )
{
    if( !Parent )
      THROW( "Parent element NULL" );
    
    XMLElement* Child = Parent->FirstChildElement( ChildName.c_str() );
    
    if( !Child )
      THROW( "Cannot find element <" + ChildName + "> inside <" + Parent->Name() + ">" );
    
    return Child;
}

// -----------------------------------------------------------------------------

// automation for string attributes in XML
string GetRequiredStringAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    return Attribute->Value();
}

// -----------------------------------------------------------------------------

// automation for integer attributes in XML
int GetRequiredIntegerAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    // attempt integer conversion
    int Number = 0;
    XMLError ErrorCode = Element->QueryIntAttribute( AtributeName.c_str(), &Number );
    
    if( ErrorCode != XML_SUCCESS )
      THROW( "Attribute '" + AtributeName + "' inside <" + Element->Name() + "> must be an integer number" );
    
    return Number;
}

// -----------------------------------------------------------------------------

// automation for yes/no attributes in XML
bool GetRequiredYesNoAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      THROW( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      THROW( "Cannot find attribute '" + AtributeName + "' inside <" + Element->Name() + ">" );
    
    if( ToLowerCase( Attribute->Value() ) == "yes" )
      return true;
    
    if( ToLowerCase( Attribute->Value() ) == "no" )
      return false;
    
    THROW( "Attribute '" + AtributeName + "' inside <" + Element->Name() + "> must be either 'yes' or 'no'" );
}


// =============================================================================
//      XML LOAD FUNCTIONS
// =============================================================================


void LoadKey( SDL_Keycode* LoadedKey, XMLElement* Parent, const string& KeyName )
{
    XMLElement* KeyElement = Parent->FirstChildElement( KeyName.c_str() );
    
    // controls may be unmapped
    if( KeyElement )
    {
        string KeyString = GetRequiredStringAttribute( KeyElement, "key" );
        *LoadedKey = SDL_GetKeyFromName( KeyString.c_str() );
    }
    
    else *LoadedKey = -1;
}

// -----------------------------------------------------------------------------

void LoadJoystickControl( JoystickControl* LoadedControl, XMLElement* Parent, const string& ControlName )
{
    XMLElement* ControlElement = Parent->FirstChildElement( ControlName.c_str() );
    
    // controls may be unmapped
    if( !ControlElement )
    {
        LoadedControl->IsAxis = false;
        LoadedControl->IsHat = false;
        LoadedControl->ButtonIndex = -1;
        return;
    }
    
    // when mapped, must be either a button or an axis
    const XMLAttribute* ButtonAttribute = ControlElement->FindAttribute( "button" );
    const XMLAttribute* AxisAttribute = ControlElement->FindAttribute( "axis" );
    const XMLAttribute* HatAttribute = ControlElement->FindAttribute( "hat" );
    
    if( ButtonAttribute )
    {
        LoadedControl->IsAxis = false;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "button", &LoadedControl->ButtonIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'button' in <") + ControlElement->Name() + "> must be a number" );
    }
    
    else if( AxisAttribute )
    {
        LoadedControl->IsAxis = true;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "axis", &LoadedControl->AxisIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'axis' in <") + ControlElement->Name() + "> must be a number" );
        
        // for an axis, it is mandatory to indicate a direction
        string AxisDirection = GetRequiredStringAttribute( ControlElement, "direction" );
        
        if( ToLowerCase( AxisDirection ) == "minus" )
          LoadedControl->AxisPositive = false;
        
        else if( ToLowerCase( AxisDirection ) == "plus" )
          LoadedControl->AxisPositive = true;
        
        else
          THROW( "Axis direction must be either 'plus' or 'minus'" );
    }
    
    else if( HatAttribute )
    {
        LoadedControl->IsHat = true;
        XMLError ErrorCode = ControlElement->QueryIntAttribute( "hat", &LoadedControl->HatIndex );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( string("Attribute 'hat' in <") + ControlElement->Name() + "> must be a number" );
        
        // for a hat, it is mandatory to indicate a direction
        string HatDirection = GetRequiredStringAttribute( ControlElement, "direction" );
        HatDirection = ToLowerCase( HatDirection );
        
        if( HatDirection == "left" )
          LoadedControl->HatDirection = SDL_HAT_LEFT;
        
        else if( HatDirection == "right" )
          LoadedControl->HatDirection = SDL_HAT_RIGHT;
        
        else if( HatDirection == "up" )
          LoadedControl->HatDirection = SDL_HAT_UP;
        
        else if( HatDirection == "down" )
          LoadedControl->HatDirection = SDL_HAT_DOWN;
        
        else
          THROW( "Hat direction must be one of: 'left', 'right', 'up' or 'down'" );
    }
    
    else
      THROW( string("For a joystick, element <") + ControlElement->Name() + "> must include 'button', 'axis' or 'hat' attribute" );
}

// -----------------------------------------------------------------------------

void LoadControls( const std::string& FilePath )
{
    LOG( "Loading controls from \"" + FilePath + "\"" );
    
    try
    {
        // load file and parse it as XML
        XMLDocument FileDoc;
        XMLError ErrorCode = FileDoc.LoadFile( FilePath.c_str() );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( "Cannot read XML from file path " + FilePath );
          
        // obtain XML root
        XMLElement* ControlsRoot = FileDoc.FirstChildElement( "controls" );
        
        if( !ControlsRoot )
          THROW( "Cannot find <controls> root element" );
        
        // check document version number
        int Version = GetRequiredIntegerAttribute( ControlsRoot, "version" );
        
        if( Version < 1 || Version > 2 )
          THROW( "Document version number is" + to_string( Version ) + ", only versions 1 and 2 are supported" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // first read the keyboard profile
        // (it must exist and be unique)
        XMLElement* KeyboardRoot = GetRequiredElement( ControlsRoot, "keyboard" );
        
        if( KeyboardRoot->NextSiblingElement( "keyboard" ) )
          THROW( "There can only be 1 keyboard mapping" );
        
        // load directions
        KeyboardMapping& KeyboardProfile = Gamepads.GetKeyboardProfile();
        LoadKey( &KeyboardProfile.Left , KeyboardRoot, "left"  );
        LoadKey( &KeyboardProfile.Right, KeyboardRoot, "right" );
        LoadKey( &KeyboardProfile.Up   , KeyboardRoot, "up"    );
        LoadKey( &KeyboardProfile.Down , KeyboardRoot, "down"  );
        
        // load buttons
        LoadKey( &KeyboardProfile.ButtonA, KeyboardRoot, "button-a" );
        LoadKey( &KeyboardProfile.ButtonB, KeyboardRoot, "button-b" );
        LoadKey( &KeyboardProfile.ButtonX, KeyboardRoot, "button-x" );
        LoadKey( &KeyboardProfile.ButtonY, KeyboardRoot, "button-y" );
        LoadKey( &KeyboardProfile.ButtonL, KeyboardRoot, "button-l" );
        LoadKey( &KeyboardProfile.ButtonR, KeyboardRoot, "button-r" );
        LoadKey( &KeyboardProfile.ButtonStart, KeyboardRoot, "button-start" );        
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // now read any listed joystick profiles
        XMLElement* JoystickRoot = ControlsRoot->FirstChildElement( "joystick" );
        
        while( JoystickRoot )
        {
            // read joystick profile nickname
            string ProfileName = GetRequiredStringAttribute( JoystickRoot, "nickname" );
            
            if( ProfileName == "")
              THROW( "Profile name cannot be empty" );
            
            // read joystick device name
            XMLElement* JoystickNameElement = GetRequiredElement( JoystickRoot, "name" );
            string JoystickName = "";
            
            if( JoystickNameElement->GetText() )
              JoystickName = JoystickNameElement->GetText();
            
            // read joystick GUID
            XMLElement* GUIDElement = GetRequiredElement( JoystickRoot, "guid" );
            
            if( !GUIDElement->GetText() )
              THROW( "Joystick GUID cannot be empty" );
            
            string GUIDString = GUIDElement->GetText();
            
            // validate and convert GUID
            if( !GUIDStringIsValid( GUIDString ) )
              THROW( "Joystick GUID is not valid" );
            
            SDL_JoystickGUID GUID = SDL_JoystickGetGUIDFromString( GUIDString.c_str() );
            
            // fill in basic profile info
            JoystickMapping* JoystickProfile = new JoystickMapping;
            JoystickProfile->GUID = GUID;
            JoystickProfile->ProfileName = ProfileName;
            JoystickProfile->JoystickName = JoystickName;
            
            // load directions
            LoadJoystickControl( &JoystickProfile->Left , JoystickRoot, "left"  );
            LoadJoystickControl( &JoystickProfile->Right, JoystickRoot, "right" );
            LoadJoystickControl( &JoystickProfile->Up   , JoystickRoot, "up"    );
            LoadJoystickControl( &JoystickProfile->Down , JoystickRoot, "down"  );
            
            // load buttons
            LoadJoystickControl( &JoystickProfile->ButtonA, JoystickRoot, "button-a" );
            LoadJoystickControl( &JoystickProfile->ButtonB, JoystickRoot, "button-b" );
            LoadJoystickControl( &JoystickProfile->ButtonX, JoystickRoot, "button-x" );
            LoadJoystickControl( &JoystickProfile->ButtonY, JoystickRoot, "button-y" );
            LoadJoystickControl( &JoystickProfile->ButtonL, JoystickRoot, "button-l" );
            LoadJoystickControl( &JoystickProfile->ButtonR, JoystickRoot, "button-r" );
            LoadJoystickControl( &JoystickProfile->ButtonStart, JoystickRoot, "button-start" );
            
            // create a joystick mapping, replacing the previous
            Gamepads.AddJoystickProfile( GUID, JoystickProfile );
            
            // advance to next joystick
            JoystickRoot = JoystickRoot->NextSiblingElement( "joystick" );
        }
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "Cannot load controls file: " + string(e.what()) );
        
        string Message = Texts( TextIDs::Errors_LoadControls_Label ) + string(e.what()) + "\n";
        Message += Texts( TextIDs::Errors_LoadControls_SetDefaults );
        
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_WARNING,
            "Error",
            Message.c_str()
        );
        
        Gamepads.SetDefaultProfiles();
    }
}


// =============================================================================
//      HANDLING SETTINGS FROM XML FILE
// =============================================================================


// we need to always have this as backup
// if the XML does not exist or has errors
void SetDefaultSettings()
{
    LOG( "Applying default settings" );
    
    // video configuration
    SetWindowZoom( 1 );
    
    // audio configuration
    Audio.SetMute( false );
    Audio.SetOutputVolume( 1.0 );
    
    // unloaded cartridge
    Console.UnloadCartridge();
    
    // unloaded memory card
    Console.UnloadMemoryCard();
    
    // set keyboard for first gamepad
    Console.SetGamepadConnection( 0, true );
    Gamepads.MappedGamepads[ 0 ].Type = DeviceTypes::Keyboard;
    
    // set no device for the rest of gamepads
    for( int i = 1; i < Constants::GamepadPorts; i++ )
    {
        Console.SetGamepadConnection( i, false );
        Gamepads.MappedGamepads[ i ].Type = DeviceTypes::NoDevice;
    }
    
    // set automatic memory card handling
    Emulator.SetCardHandling( true );
    
    // set default slot for savestates
    SavestatesSlot = 1;
    
    // set default load folders
    LastCartridgeDirectory = EmulatorFolder;
    LastMemoryCardDirectory = EmulatorFolder;
    
    // clear lists of recents
    RecentCartridgePaths.clear();
    RecentMemoryCardPaths.clear();
}

// -----------------------------------------------------------------------------

void LoadSettings( const string& FilePath )
{
    LOG( "Loading settings from \"" + FilePath + "\"" );
    
    try
    {
        // load file and parse it as XML
        XMLDocument FileDoc;
        XMLError ErrorCode = FileDoc.LoadFile( FilePath.c_str() );
        
        if( ErrorCode != XML_SUCCESS )
          THROW( "Cannot read XML from file path " + FilePath );
          
        // obtain XML root
        XMLElement* SettingsRoot = FileDoc.FirstChildElement( "settings" );
        
        if( !SettingsRoot )
          THROW( "Cannot find <settings> root element" );
        
        // check document version number
        int Version = GetRequiredIntegerAttribute( SettingsRoot, "version" );
        
        if( Version < 1 || Version > 6 )
          THROW( "Document version number is" + to_string( Version ) + ", only versions 1 through 6 are supported" );
        
        // load language settings (optional)
        XMLElement* LanguageElement = SettingsRoot->FirstChildElement( "language" );
        
        if( LanguageElement && LanguageElement->GetText() )
        {
            string LanguageName = ToLowerCase( LanguageElement->GetText() );
            
            if( LanguageName == "spanish" )
              SetLanguage( "Spanish" );
            else if( LanguageName == "english" )
              SetLanguage( "English" );
            else
            {
                DelayedMessageBox
                (
                    SDL_MESSAGEBOX_WARNING,
                    Texts( TextIDs::Errors_InvalidLanguage_Title ),
                    Texts( TextIDs::Errors_InvalidLanguage_Label )
                );
                
                SetLanguage( "English" );
            }
        }
        
        // load BIOS location (optional)
        XMLElement* BiosElement = SettingsRoot->FirstChildElement( "bios" );
        
        if( BiosElement )
          BiosFileName = GetRequiredStringAttribute( BiosElement, "file" );
        
        // load video settings
        XMLElement* VideoElement = GetRequiredElement( SettingsRoot, "video" );
        int WindowedZoomFactor = GetRequiredIntegerAttribute( VideoElement, "size" );
        bool FullScreen = GetRequiredYesNoAttribute( VideoElement, "fullscreen" );
        
        // apply video settings
        if( FullScreen ) SetFullScreen();
        else SetWindowZoom( WindowedZoomFactor );
        
        // load audio settings
        XMLElement* AudioElement = GetRequiredElement( SettingsRoot, "audio" );
        bool Mute = GetRequiredYesNoAttribute( AudioElement, "mute" );
        int Volume = GetRequiredIntegerAttribute( AudioElement, "volume" );
        Clamp( Volume, 0, 100 );
        
        // apply audio settings
        Audio.SetMute( Mute );
        Audio.SetOutputVolume( Volume / 100.0 );
        
        // load audio buffers settings
        XMLElement* AudioBuffersElement = GetRequiredElement( SettingsRoot, "audio-buffers" );
        int NumberOfBuffers = GetRequiredIntegerAttribute( AudioBuffersElement, "number" );
        Clamp( NumberOfBuffers, MIN_BUFFERS, MAX_BUFFERS );
        
        // apply audio buffers settings
        Audio.SetNumberOfBuffers( NumberOfBuffers );
        
        // configure gamepads
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // access the Nth gamepad element
            string GamepadElementName = string("gamepad-") + to_string( Gamepad+1 );
            XMLElement* GamepadRoot = GetRequiredElement( SettingsRoot, GamepadElementName.c_str() );
            
            // preemptively leave the gamepad unmapped and disconnected
            // unless a valid profile is found for it later
            Console.SetGamepadConnection( Gamepad, false );
            Gamepads.MappedGamepads[ Gamepad ].Type = DeviceTypes::NoDevice;
            
            // read profile name
            string ProfileName = GetRequiredStringAttribute( GamepadRoot, "profile" );
            
            // CASE A: no profile; the gamepad is disconnected
            if( ToLowerCase( ProfileName ) == "none" )
            {
                Gamepads.MappedGamepads[ Gamepad ].Type = DeviceTypes::NoDevice;
                continue;
            }
            
            // CASE B: the keyboard profile
            if( ToLowerCase( ProfileName ) == "keyboard" )
            {
                Gamepads.MappedGamepads[ Gamepad ].Type = DeviceTypes::Keyboard;
                Console.SetGamepadConnection( Gamepad, true );
                continue;
            }
            
            // CASE C: other names are considered joystick profiles
            JoystickMapping* JoystickProfile = Gamepads.GetJoystickProfile( ProfileName );
            
            if( JoystickProfile )
            {
                Gamepads.MappedGamepads[ Gamepad ].Type = DeviceTypes::Joystick;
                Gamepads.MappedGamepads[ Gamepad ].GUID = JoystickProfile->GUID;
                Console.SetGamepadConnection( Gamepad, true );
            }
            
            // warn when a profile name is not found
            if( Gamepads.MappedGamepads[ Gamepad ].Type != DeviceTypes::Joystick )
            {
                string Message = Texts( TextIDs::Errors_InvalidDevice_CannotFind ) + string("\"") + ProfileName + "\".\n";
                Message += Texts( TextIDs::Errors_InvalidDevice_Gamepad ) + to_string( Gamepad+1 );
                Message += Texts( TextIDs::Errors_InvalidDevice_SetNoDevice );
                
                DelayedMessageBox
                (
                    SDL_MESSAGEBOX_WARNING,
                    Texts( TextIDs::Errors_InvalidDevice_Title ),
                    Message.c_str()
                );
            }
        }
        
        // now that profiles have been associated to gamepad ports,
        // we need to detect joysticks and assign them to gamepads
        Gamepads.AssignInputDevices();
        
        // read memory card handling mode (optional)
        XMLElement* MemCardElement = SettingsRoot->FirstChildElement( "memory-card" );
        
        if( MemCardElement )
        {
            bool AutoCards = GetRequiredYesNoAttribute( MemCardElement, "automatic" );
            Emulator.SetCardHandling( AutoCards );
        }
        
        // save current savestate slot (optional)
        XMLElement* SavestatesElement = SettingsRoot->FirstChildElement( "savestates" );
        SavestatesSlot = 1;
        
        if( SavestatesElement )
        {
            int Slot = GetRequiredIntegerAttribute( SavestatesElement, "slot" );
            SavestatesSlot = Slot;
            
            // limit to the valid range
            if( Slot < 1 ) Slot = 1;
            if( Slot > 4 ) Slot = 4;
        }
        
        // read load folders
        XMLElement* LoadFoldersRoot = GetRequiredElement( SettingsRoot, "load-folders" );
        XMLElement* CartridgeFolder = GetRequiredElement( LoadFoldersRoot, "cartridges" );
        XMLElement* MemoryCardFolder = GetRequiredElement( LoadFoldersRoot, "memory-cards" );
        
        LastCartridgeDirectory = GetRequiredStringAttribute( CartridgeFolder, "path" );
        LastMemoryCardDirectory = GetRequiredStringAttribute( MemoryCardFolder, "path" );
        
        // when empty, set default load folders
        if( LastCartridgeDirectory  == "" ) LastCartridgeDirectory  = EmulatorFolder;
        if( LastMemoryCardDirectory == "" ) LastMemoryCardDirectory = EmulatorFolder;
        
        // load recent cartridges
        RecentCartridgePaths.clear();
        XMLElement* CartridgesRoot = GetRequiredElement( SettingsRoot, "recent-cartridges" );
        XMLElement* CartridgeElement = CartridgesRoot->FirstChildElement( "cartridge" );
        
        while( CartridgeElement )
        {
            string CartridgePath = GetRequiredStringAttribute( CartridgeElement, "path" );
            RecentCartridgePaths.push_back( CartridgePath );
            
            CartridgeElement = CartridgeElement->NextSiblingElement( "cartridge" );
        }
        
        // load recent memory cards
        RecentMemoryCardPaths.clear();
        XMLElement* MemoryCardsRoot = GetRequiredElement( SettingsRoot, "recent-memory-cards" );
        XMLElement* MemoryCardElement = MemoryCardsRoot->FirstChildElement( "memory-card" );
        
        while( MemoryCardElement )
        {
            string MemoryCardPath = GetRequiredStringAttribute( MemoryCardElement, "path" );
            RecentMemoryCardPaths.push_back( MemoryCardPath );
            
            MemoryCardElement = MemoryCardElement->NextSiblingElement( "memory-card" );
        }
        
        // check both lists
        CheckCartridgePaths();
        CheckMemoryCardPaths();
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "Cannot load settings file: " + string(e.what()) );
        
        string Message = Texts( TextIDs::Errors_LoadSettings_Label ) + string(e.what()) + "\n";
        Message += Texts( TextIDs::Errors_LoadSettings_SetDefaults );
        
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_WARNING,
            "Error",
            Message.c_str()
        );
        
        SetDefaultSettings();
    }
}

// -----------------------------------------------------------------------------

void SaveSettings( const string& FilePath )
{
    LOG( "Saving settings to \"" + FilePath + "\"" );
    
    try
    {
        // create a document and its root element
        XMLDocument CreatedDoc;
        XMLElement* SettingsRoot = CreatedDoc.NewElement( "settings" );
        SettingsRoot->SetAttribute( "version", 6 );
        CreatedDoc.LinkEndChild( SettingsRoot );
        
        // save language
        string LanguageName = (CurrentLanguage == &LanguageEnglish[0]? "english" : "spanish");
        XMLElement* LanguageElement = CreatedDoc.NewElement( "language" );
        LanguageElement->SetText( LanguageName.c_str() );
        SettingsRoot->LinkEndChild( LanguageElement );
        
        // save bios file
        XMLElement* BiosElement = CreatedDoc.NewElement( "bios" );
        BiosElement->SetAttribute( "file", BiosFileName.c_str() );
        SettingsRoot->LinkEndChild( BiosElement );
        
        // save video settings
        XMLElement* VideoElement = CreatedDoc.NewElement( "video" );
        VideoElement->SetAttribute( "size", (unsigned)Video.GetWindowZoom() );
        VideoElement->SetAttribute( "fullscreen", Video.IsFullScreen()? "yes" : "no" );
        SettingsRoot->LinkEndChild( VideoElement );
        
        // save audio settings
        XMLElement* AudioElement = CreatedDoc.NewElement( "audio" );
        AudioElement->SetAttribute( "mute", Audio.IsMuted()? "yes" : "no" );
        AudioElement->SetAttribute( "volume", (unsigned)(100.0 * Audio.GetOutputVolume()) );
        SettingsRoot->LinkEndChild( AudioElement );
        
        // save audio buffers settings
        XMLElement* AudioBuffersElement = CreatedDoc.NewElement( "audio-buffers" );
        AudioBuffersElement->SetAttribute( "number", Audio.GetNumberOfBuffers() );
        SettingsRoot->LinkEndChild( AudioBuffersElement );
        
        // save gamepad profiles
        for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
        {
            // determine profile name
            DeviceInfo MappedDevice = Gamepads.MappedGamepads[ Gamepad ];
            string ProfileName;
            
            if( MappedDevice.Type == DeviceTypes::NoDevice )
              ProfileName = "none";
            
            else if( MappedDevice.Type == DeviceTypes::Keyboard )
              ProfileName = "keyboard";
            
            else  // it is a joystick
            {
                // obtain the applicable joystick profile
                JoystickMapping* JoystickProfile = Gamepads.GetJoystickProfile( MappedDevice.GUID );
                ProfileName = (JoystickProfile? JoystickProfile->ProfileName : "none");
            }
            
            // now save it
            string ElementName = "gamepad-" + to_string(Gamepad+1);
            XMLElement* GamepadElement = CreatedDoc.NewElement( ElementName.c_str() );
            GamepadElement->SetAttribute( "profile", ProfileName.c_str() );
            SettingsRoot->LinkEndChild( GamepadElement );
        }
        
        // save memory card handling mode
        XMLElement* MemCardElement = CreatedDoc.NewElement( "memory-card" );
        SettingsRoot->LinkEndChild( MemCardElement );
        MemCardElement->SetAttribute( "automatic", Emulator.IsCardHandlingAuto()? "yes" : "no" );
        
        // save current savestate slot
        XMLElement* SavestatesElement = CreatedDoc.NewElement( "savestates" );
        SettingsRoot->LinkEndChild( SavestatesElement );
        SavestatesElement->SetAttribute( "slot", SavestatesSlot );
        
        // save load folders
        XMLElement* FoldersRoot = CreatedDoc.NewElement( "load-folders" );
        SettingsRoot->LinkEndChild( FoldersRoot );
        
        XMLElement* CartridgeFolderElement = CreatedDoc.NewElement( "cartridges" );
        CartridgeFolderElement->SetAttribute( "path", LastCartridgeDirectory.c_str() );
        FoldersRoot->LinkEndChild( CartridgeFolderElement );
        
        XMLElement* CardFolderElement = CreatedDoc.NewElement( "memory-cards" );
        CardFolderElement->SetAttribute( "path", LastMemoryCardDirectory.c_str() );
        FoldersRoot->LinkEndChild( CardFolderElement );
        
        // save recent cartridge paths
        XMLElement* CartridgesRoot = CreatedDoc.NewElement( "recent-cartridges" );
        SettingsRoot->LinkEndChild( CartridgesRoot );
        
        for( string Path: RecentCartridgePaths )
        {
            XMLElement* PathElement = CreatedDoc.NewElement( "cartridge" );
            PathElement->SetAttribute( "path", Path.c_str() );
            CartridgesRoot->LinkEndChild( PathElement );            
        }
        
        // save recent memory card paths
        XMLElement* MemoryCardsRoot = CreatedDoc.NewElement( "recent-memory-cards" );
        SettingsRoot->LinkEndChild( MemoryCardsRoot );
        
        for( string Path: RecentMemoryCardPaths )
        {
            XMLElement* PathElement = CreatedDoc.NewElement( "memory-card" );
            PathElement->SetAttribute( "path", Path.c_str() );
            MemoryCardsRoot->LinkEndChild( PathElement );            
        }
        
        // save XML document to the designated file
        CreatedDoc.SaveFile( FilePath.c_str() );
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "Cannot save settings file: " + string(e.what()) );
        
        string Message = Texts( TextIDs::Errors_SaveSettings_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}
