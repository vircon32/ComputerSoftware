// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    #include "DesktopInfrastructure/StringFunctions.hpp"
    
    // include controls editor headers
    #include "Controls.hpp"
    #include "Globals.hpp"
    #include "Languages.hpp"
    
    // include C/C++ headers
    #include <set>              // [ C++ STL ] Sets
    
    // include TinyXML2 headers
    #include <tinyxml2.h>       // [ TinyXML2 ] Main header
    
    // declare used namespaces
    using namespace std;
    using namespace tinyxml2;
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR INPUT MAPPINGS
// =============================================================================


JoystickControl::JoystickControl()
{
    IsAxis = false;
    IsHat = false;
    ButtonIndex = -1;
    AxisIndex = -1;
    HatIndex = -1;
    AxisPositive = true;
    HatDirection = SDL_HAT_CENTERED;
}


// =============================================================================
//     OPERATION WITH GUIDS
// =============================================================================


bool operator==( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 )
{
    return !memcmp( &GUID1, &GUID2, sizeof(SDL_JoystickGUID) );
}

// -----------------------------------------------------------------------------

bool operator!=( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 )
{
    return memcmp( &GUID1, &GUID2, sizeof(SDL_JoystickGUID) );
}

// -----------------------------------------------------------------------------

bool operator<( const SDL_JoystickGUID& GUID1, const SDL_JoystickGUID& GUID2 )
{
    int Result = memcmp( &GUID1, &GUID2, sizeof(SDL_JoystickGUID) );
    return (Result < 0);
}

// -----------------------------------------------------------------------------

string GUIDToString( SDL_JoystickGUID GUID )
{
    char GUIDString[ 35 ];
    SDL_JoystickGetGUIDString( GUID, GUIDString, 34 );  
    return GUIDString;  
}

// -----------------------------------------------------------------------------

bool GUIDStringIsValid( const string& GUIDString )
{
    // length must be even and no greater than 32 characters
    if( GUIDString.size() > 32 ) return false;
    if( GUIDString.size() &  1 ) return false;
    
    // characters must be hexadecimal and lowercase
    for( char c: GUIDString )
    {
        if( isdigit( c ) ) continue;
        if( isupper( c ) ) return false;
        if( c < 'a' && c > 'f' ) return false;
    }
    
    return true;
}


// =============================================================================
//      GLOBALS FOR INPUT PROFILES
// =============================================================================


// all currently connected joysticks
map< SDL_JoystickID, SDL_JoystickGUID > ConnectedJoysticks;

// all of our available mappings
KeyboardMapping KeyboardProfile;
map< SDL_JoystickGUID, JoystickMapping* > JoystickProfiles;

// the currently selected joystick profile
// (when NULL, the keyboard mapping is selected)
JoystickMapping* SelectedProfile = nullptr;

// controls being mapped
// (when not NULL, events are redirected to their definition)
SDL_Keycode* KeyBeingMapped = nullptr;
JoystickControl* ControlBeingMapped = nullptr;


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
//      RESETTING INPUT MAPPINGS
// =============================================================================


void SetDefaultControls()
{
    // first delete any joystick profiles
    for( auto Pair: JoystickProfiles )
      delete Pair.second;
    
    JoystickProfiles.clear();
    
    // set the default keyboard profile
    KeyboardProfile.Left = SDLK_LEFT;
    KeyboardProfile.Right = SDLK_RIGHT;
    KeyboardProfile.Up = SDLK_UP;
    KeyboardProfile.Down = SDLK_DOWN;
    
    KeyboardProfile.ButtonA = SDLK_x;
    KeyboardProfile.ButtonB = SDLK_z;
    KeyboardProfile.ButtonX = SDLK_s;
    KeyboardProfile.ButtonY = SDLK_a;
    KeyboardProfile.ButtonL = SDLK_q;
    KeyboardProfile.ButtonR = SDLK_w;
    
    KeyboardProfile.ButtonStart = SDLK_RETURN;
    
    // keyboard is the only selectable profile now
    SelectedProfile = nullptr;
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

void LoadControls( const string& FilePath )
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
            JoystickProfiles[ GUID ] = JoystickProfile;
            
            // advance to next joystick
            JoystickRoot = JoystickRoot->NextSiblingElement( "joystick" );
        }
    }
    
    // as backup, set our default configuration
    catch( exception& e )
    {
        LOG( "Error loading controls: " + string(e.what()) );
        string ErrorMessage = Texts( TextIDs::Errors_LoadControls_Label ) + string(e.what()) + "\n";
        ErrorMessage += Texts( TextIDs::Errors_LoadControls_SetDefaults );
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error", ErrorMessage.c_str(), nullptr );
        
        SetDefaultControls();
    }
}


// =============================================================================
//      XML SAVE FUNCTIONS
// =============================================================================


void SaveKey( SDL_Keycode* SavedKey, XMLElement* Parent, const string& KeyName )
{
    if( *SavedKey < 0 )
      return;
    
    XMLElement* KeyElement = Parent->GetDocument()->NewElement( KeyName.c_str() );
    Parent->InsertEndChild( KeyElement );
    KeyElement->SetAttribute( "key", SDL_GetKeyName( *SavedKey ) );
}

// -----------------------------------------------------------------------------

void SaveJoystickControl( JoystickControl* SavedControl, XMLElement* Parent, const string& ControlName )
{
    // the control may be unmapped
    if( !SavedControl->IsAxis && !SavedControl->IsHat && SavedControl->ButtonIndex < 0 )
      return;
    
    XMLElement* ControlElement = Parent->GetDocument()->NewElement( ControlName.c_str() );
    Parent->InsertEndChild( ControlElement );
    
    if( SavedControl->IsAxis )
    {
        ControlElement->SetAttribute( "axis", (int)SavedControl->AxisIndex );
        ControlElement->SetAttribute( "direction", SavedControl->AxisPositive? "plus" : "minus" );
    }
    
    else if( SavedControl->IsHat )
    {
        ControlElement->SetAttribute( "hat", (int)SavedControl->HatIndex );
        
        string DirectionText = "";
        
        if( SavedControl->HatDirection & SDL_HAT_LEFT )
          DirectionText = "left";
        
        else if( SavedControl->HatDirection & SDL_HAT_RIGHT )
          DirectionText = "right";
        
        else if( SavedControl->HatDirection & SDL_HAT_UP )
          DirectionText = "up";
        
        else if( SavedControl->HatDirection & SDL_HAT_DOWN )
          DirectionText = "down";
        
        ControlElement->SetAttribute( "direction", DirectionText.c_str() );
    }
    
    else
    {
        ControlElement->SetAttribute( "button", (int)SavedControl->ButtonIndex );
    }
}

// -----------------------------------------------------------------------------

void SaveControls( const std::string& FilePath )
{
    try
    {
        // create a document and its root element
        XMLDocument CreatedDoc;
        XMLElement* ControlsRoot = CreatedDoc.NewElement( "controls" );
        ControlsRoot->SetAttribute( "version", 2 );
        CreatedDoc.LinkEndChild( ControlsRoot );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // save the keyboard mapping 
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // create root element
        XMLElement* KeyboardRoot = CreatedDoc.NewElement( "keyboard" );
        ControlsRoot->InsertEndChild( KeyboardRoot );
        
        // save directions
        SaveKey( &KeyboardProfile.Left , KeyboardRoot, "left"  );
        SaveKey( &KeyboardProfile.Right, KeyboardRoot, "right" );
        SaveKey( &KeyboardProfile.Up   , KeyboardRoot, "up"    );
        SaveKey( &KeyboardProfile.Down , KeyboardRoot, "down"  );
        
        // save buttons
        SaveKey( &KeyboardProfile.ButtonA, KeyboardRoot, "button-a" );
        SaveKey( &KeyboardProfile.ButtonB, KeyboardRoot, "button-b" );
        SaveKey( &KeyboardProfile.ButtonX, KeyboardRoot, "button-x" );
        SaveKey( &KeyboardProfile.ButtonY, KeyboardRoot, "button-y" );
        SaveKey( &KeyboardProfile.ButtonL, KeyboardRoot, "button-l" );
        SaveKey( &KeyboardProfile.ButtonR, KeyboardRoot, "button-r" );
        SaveKey( &KeyboardProfile.ButtonStart, KeyboardRoot, "button-start" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // save every joystick mapping 
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        for( auto Pair: JoystickProfiles )
        {
            JoystickMapping* JoystickProfile = Pair.second;
            
            // create root element
            XMLElement* JoystickRoot = CreatedDoc.NewElement( "joystick" );
            JoystickRoot->SetAttribute( "nickname", JoystickProfile->ProfileName.c_str() );
            ControlsRoot->InsertEndChild( JoystickRoot );
            
            // create element for GUID
            XMLElement* GUIDElement = CreatedDoc.NewElement( "guid" );
            string GUIDString = GUIDToString( JoystickProfile->GUID );
            GUIDElement->SetText( GUIDString.c_str() );
            JoystickRoot->InsertEndChild( GUIDElement );
            
            // create element for joystick name
            XMLElement* NameElement = CreatedDoc.NewElement( "name" );
            NameElement->SetText( JoystickProfile->JoystickName.c_str() );
            JoystickRoot->InsertEndChild( NameElement );
            
            // save directions
            SaveJoystickControl( &JoystickProfile->Left , JoystickRoot, "left"  );
            SaveJoystickControl( &JoystickProfile->Right, JoystickRoot, "right" );
            SaveJoystickControl( &JoystickProfile->Up   , JoystickRoot, "up"    );
            SaveJoystickControl( &JoystickProfile->Down , JoystickRoot, "down"  );
            
            // save buttons
            SaveJoystickControl( &JoystickProfile->ButtonA, JoystickRoot, "button-a" );
            SaveJoystickControl( &JoystickProfile->ButtonB, JoystickRoot, "button-b" );
            SaveJoystickControl( &JoystickProfile->ButtonX, JoystickRoot, "button-x" );
            SaveJoystickControl( &JoystickProfile->ButtonY, JoystickRoot, "button-y" );
            SaveJoystickControl( &JoystickProfile->ButtonL, JoystickRoot, "button-l" );
            SaveJoystickControl( &JoystickProfile->ButtonR, JoystickRoot, "button-r" );
            SaveJoystickControl( &JoystickProfile->ButtonStart, JoystickRoot, "button-start" );
        }
        
        // save XML document to the designated file
        CreatedDoc.SaveFile( FilePath.c_str() );
    }
    
    // no backup; just report to the user
    catch( exception& e )
    {
        LOG( "Error saving controls: " + string(e.what()) );
        string ErrorMessage = Texts( TextIDs::Errors_SaveControls_Label ) + string(e.what());
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error", ErrorMessage.c_str(), nullptr );
    }
}
