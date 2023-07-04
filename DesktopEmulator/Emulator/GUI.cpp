// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/OpenGL2DContext.hpp"
    #include "../DesktopInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "GUI.hpp"
    #include "V32Emulator.hpp"
    #include "Globals.hpp"
    #include "Settings.hpp"
    #include "Languages.hpp"
    
    // include osdialog headers
    #include <osdialog/osdialog.h>
    
    // include libpng headers
    #include <png.h>
    
    // include C/C++ headers
    #include <time.h>               // [ ANSI C ] Time and date
    #include <stdexcept>            // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      WRAPPERS TO LOAD AND SAVE FILES
// =============================================================================
// these are just wrappers to use the dialog functions
// in OSDialog's library and adapt them to our own use


string GetLoadFilePath( const char* Filters, const std::string& Directory = EmulatorFolder )
{
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    // show the dialog with the requested filter
    osdialog_filters* ParsedFilters = osdialog_filters_parse( Filters );
    char* FilePath = osdialog_file( OSDIALOG_OPEN, Directory.c_str(), "", ParsedFilters );
    osdialog_filters_free( ParsedFilters );
    
    // resume emulation if needed
    if( WasRunning )
      Vircon.Resume();
    
    // when cancelled, return empty string
    if( !FilePath )
      return "";
    
    // we need to save the result in a string
    string Result = FilePath;
    free( FilePath );
    return Result;
}

// -----------------------------------------------------------------------------

string GetSaveFilePath( const char* Filters, const std::string& Directory = EmulatorFolder )
{
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    // show the dialog with the requested filter
    osdialog_filters* ParsedFilters = osdialog_filters_parse( Filters );
    char* FilePath = osdialog_file( OSDIALOG_SAVE, Directory.c_str(), "", ParsedFilters );
    osdialog_filters_free( ParsedFilters );
    
    // resume emulation if needed
    if( WasRunning )
      Vircon.Resume();
    
    // when cancelled, return empty string
    if( !FilePath )
      return "";
    
    // we need to save the result in a string
    string Result = FilePath;
    free( FilePath );
    return Result;
}


// =============================================================================
//      DELAYED MESSAGE BOX FUNCTIONS
// =============================================================================
// these are needed because in full screen, showing a
// regular message box may blocks the program. This is
// avoided by waiting until the frame finishes drawing

bool MessageBoxPending = false;
Uint32 MessageBoxFlags;
const char *MessageBoxTitle, *MessageBoxMessage;

// -----------------------------------------------------------------------------

void DelayedMessageBox( Uint32 Flags, const char *Title, const char *Message )
{
    MessageBoxPending = true;
    MessageBoxFlags = Flags;
    MessageBoxTitle = Title;
    MessageBoxMessage = Message;
}

// -----------------------------------------------------------------------------

void ShowDelayedMessageBox()
{
    if( !MessageBoxPending )
      return;
    
    // check current state to restore later
    bool WasFullScreen = OpenGL2D.FullScreen;
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    if( WasFullScreen )
    {
        SetWindowZoom( 2 );
        SDL_GL_SwapWindow( OpenGL2D.Window );
    }
    
    SDL_ShowSimpleMessageBox( MessageBoxFlags, MessageBoxTitle, MessageBoxMessage, nullptr );
    MessageBoxPending = false;
    
    // restore previous state if needed
    if( WasFullScreen )
    {
        SetFullScreen();
        SDL_GL_SwapWindow( OpenGL2D.Window );
    }
    
    if( WasRunning )
      Vircon.Resume();
}


// =============================================================================
//      ADDITIONAL GUI FUNCTIONS
// =============================================================================
// these are some more complex GUI functionalities that
// are better understood as their own separate functions


void SetWindowZoom( int ZoomFactor )
{
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    // set the zoom
    OpenGL2D.SetWindowZoom( ZoomFactor );
    
    // scale ImGui
    ImGui::GetIO().FontGlobalScale = OpenGL2D.WindowedZoomFactor;
    
    // resume emulation if needed
    if( WasRunning )
      Vircon.Resume();
}

// -----------------------------------------------------------------------------

void SetFullScreen()
{
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Vircon.PowerIsOn && !Vircon.Paused;
    
    if( WasRunning )
      Vircon.Pause();
    
    // set full screen
    OpenGL2D.SetFullScreen();
    
    // scale ImGui
    ImGui::GetIO().FontGlobalScale = (float)OpenGL2D.WindowWidth / Constants::ScreenWidth;
    
    // resume emulation if needed
    if( WasRunning )
      Vircon.Resume();
}

// -----------------------------------------------------------------------------

void SaveScreenshot( const string& FilePath )
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1: Read the contents of the framebuffer object
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // allocate space for the image
    uint8_t* Pixels =  new uint8_t[ 4 * Constants::ScreenWidth * Constants::ScreenHeight ];
    uint8_t** RowPointers = new uint8_t*[ Constants::ScreenHeight ];
    
    // images on framebuffer are rendered inverted in Y, so
    // we invert the order of rows to reflect it back to normal
    for( int y = 0; y < Constants::ScreenHeight; y++ )
      RowPointers[ (Constants::ScreenHeight-1) - y ] = &Pixels[ 4 * Constants::ScreenWidth * y ];
      
    // dump content of framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, OpenGL2D.FramebufferID );
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glReadPixels( 0, 0, Constants::ScreenWidth, Constants::ScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, Pixels );
    
    // unbind framebuffer, since this function is called from GUI
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 2: Save those contents in PNG format
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // open output file
    FILE *PNGFile = fopen( FilePath.c_str(), "wb" );
    
    if( !PNGFile )
      throw runtime_error( "Cannot open output file" );
    
    // initialize PNG functions
    png_struct* PNGHandler = png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
    
    if( !PNGHandler )
      throw runtime_error( "Cannot create PNG handler" );

    png_info* PNGInfo = png_create_info_struct( PNGHandler );
    
    if( !PNGInfo )
      throw runtime_error( "Cannot write PNG information" );

    // define a callback function expected by libpng for error handling
    if( setjmp( png_jmpbuf(PNGHandler) ) )
      throw runtime_error( "Cannot initialize PNG error handling" );
      
    // begin writing
    png_init_io( PNGHandler, PNGFile );

    // define output as 8bit depth in RGBA format
    png_set_IHDR
    (
        PNGHandler,
        PNGInfo,
        Constants::ScreenWidth,
        Constants::ScreenHeight,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    
    // write basic image info
    png_write_info( PNGHandler, PNGInfo );
    
    // write the actual pixel data for all rows
    png_write_image( PNGHandler, RowPointers );
    
    // end writing
    png_write_end( PNGHandler, nullptr );
    
    // clean-up
    fclose( PNGFile );
    png_destroy_write_struct( &PNGHandler, &PNGInfo );
    
    delete Pixels;
    delete RowPointers;
}

// -----------------------------------------------------------------------------

void AddRecentCartridgePath( const string& CartridgePath )
{
    // avoid duplicates
    RecentCartridgePaths.remove( CartridgePath );
    RecentCartridgePaths.push_front( CartridgePath );
    
    // keep a maximum of 5 items
    while( RecentCartridgePaths.size() > 5 )
      RecentCartridgePaths.pop_back();
}

// -----------------------------------------------------------------------------

void AddRecentMemoryCardPath( const string& MemoryCardPath )
{
    // avoid duplicates
    RecentMemoryCardPaths.remove( MemoryCardPath );
    RecentMemoryCardPaths.push_front( MemoryCardPath );
    
    // keep a maximum of 5 items
    while( RecentMemoryCardPaths.size() > 5 )
      RecentMemoryCardPaths.pop_back();
}

// -----------------------------------------------------------------------------

void CheckCartridgePaths()
{
    for( auto Position = RecentCartridgePaths.begin(); Position != RecentCartridgePaths.end(); Position++ )
      if( !FileExists( *Position ) )
        Position = RecentCartridgePaths.erase( Position );
        
    // keep a maximum of 5 items
    while( RecentCartridgePaths.size() > 5 )
      RecentCartridgePaths.pop_back();
}

// -----------------------------------------------------------------------------

void CheckMemoryCardPaths()
{
    for( auto Position = RecentMemoryCardPaths.begin(); Position != RecentMemoryCardPaths.end(); Position++ )
      if( !FileExists( *Position ) )
        Position = RecentMemoryCardPaths.erase( Position );
    
    // keep a maximum of 5 items
    while( RecentMemoryCardPaths.size() > 5 )
      RecentMemoryCardPaths.pop_back();
}


// =============================================================================
//      ENCAPSULATED GUI FUNCTIONS
// =============================================================================
// some of the GUI functions involve files and can potentially
// throw exceptions, so we have to add some logic to wrap them
// in try/catch blocks and report errors


void GUI_CreateMemoryCard()
{
    try
    {
        string MemoryCardPath = GetSaveFilePath( "Vircon32 cards (*.memc):memc" );
        
        if( !MemoryCardPath.empty() )
        {
            // ensure path has the proper extension
            if( GetFileExtension( MemoryCardPath ) != "memc" )
              MemoryCardPath += ".memc";
            
            // create the card
            Vircon.CreateMemoryCard( MemoryCardPath );
            
            // report
            DelayedMessageBox
            (
                SDL_MESSAGEBOX_INFORMATION,
                Texts(TextIDs::Dialogs_Done),
                Texts(TextIDs::Dialogs_CardCreated_Label)
            );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_CreateCard_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_UnloadMemoryCard()
{
    try
    {
        Vircon.UnloadMemoryCard();
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_UnloadCard_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_LoadMemoryCard( string MemoryCardPath )
{
    try
    {
        if( MemoryCardPath.empty() )
          MemoryCardPath = GetLoadFilePath( "Vircon32 cards (*.memc):memc", LastMemoryCardDirectory );
        
        if( !MemoryCardPath.empty() )
        {
            LastMemoryCardDirectory = GetPathDirectory( MemoryCardPath );
            Vircon.LoadMemoryCard( MemoryCardPath );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_LoadCard_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_ChangeMemoryCard( string MemoryCardPath )
{
    try
    {
        if( MemoryCardPath.empty() )
          MemoryCardPath = GetLoadFilePath( "Vircon32 cards (*.memc):memc", LastMemoryCardDirectory );
        
        if( !MemoryCardPath.empty() )
        {
            LastMemoryCardDirectory = GetPathDirectory( MemoryCardPath );
            Vircon.UnloadMemoryCard();
            Vircon.LoadMemoryCard( MemoryCardPath );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_ChangeCard_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_UnloadCartridge()
{
    try
    {
        Vircon.UnloadCartridge();
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_UnloadCartridge_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_LoadCartridge( string CartridgePath )
{
    try
    {
        if( CartridgePath.empty() )
          CartridgePath = GetLoadFilePath( "Vircon32 roms (*.v32):v32", LastCartridgeDirectory );
        
        if( !CartridgePath.empty() )
        {
            LastCartridgeDirectory = GetPathDirectory( CartridgePath );
            
            Vircon.LoadCartridge( CartridgePath );
            Vircon.PowerOn();
            
            // fix to prevent GUI from drawing
            // on the console's framebuffer
            MouseIsOnWindow = false;
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_LoadCartridge_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_ChangeCartridge( string CartridgePath )
{
    try
    {
        if( CartridgePath.empty() )
          CartridgePath = GetLoadFilePath( "Vircon32 roms (*.v32):v32", LastCartridgeDirectory );
        
        if( !CartridgePath.empty() )
        {
            LastCartridgeDirectory = GetPathDirectory( CartridgePath );
            
            Vircon.UnloadCartridge();
            Vircon.LoadCartridge( CartridgePath );
            Vircon.PowerOn();
            
            // fix to prevent GUI from drawing
            // on the console's framebuffer
            MouseIsOnWindow = false;
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts(TextIDs::Errors_ChangeCartridge_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_SaveScreenshot( string FilePath )
{
    try
    {
        // if a path is not provided, an automatic
        // file name is created with time and date
        if( FilePath.empty() )
        {
            // obtain current time
            time_t CreationTime;
            time( &CreationTime );
            struct tm* CreationTimeInfo = localtime( &CreationTime );
            
            // determine a file name from current date and time
            // (Careful! C gives year counting from 1900)
            char FileName[ 40 ];
            
            sprintf
            (
                FileName,
                "%04d-%02d-%02d %02d.%02d.%02d.png",
                CreationTimeInfo->tm_year+1900,
                CreationTimeInfo->tm_mon+1,
                CreationTimeInfo->tm_mday,
                CreationTimeInfo->tm_hour,
                CreationTimeInfo->tm_min,
                CreationTimeInfo->tm_sec
            );
            
            // place the screenshot in the screenshots subfolder
            FilePath = EmulatorFolder + "Screenshots" + PathSeparator + FileName;
        }
        
        // now just call the screenshot save function
        SaveScreenshot( FilePath );
        
        // report success
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts(TextIDs::Dialogs_Done),
            Texts(TextIDs::Dialogs_ScreenshotSaved_Label)
        );
    }
    
    catch( exception& e )
    {
        string MessageBoxText = Texts(TextIDs::Errors_SaveScreenshot_Label) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", MessageBoxText.c_str() );
    }
}


// =============================================================================
//      SUPPORT FOR DELAYED FILE GUI ACTIONS
// =============================================================================
// Under Linux there is a risk that performing I/O
// actions related to emulator resource files will
// crash the emulator if they are performed during
// GUI processing. To prevent that, we will instead
// store the requested action to perform it after
// GUI processing has finished for the current frame

enum class DelayedFileActions
{
    None,
    UnloadCartridge,
    LoadCartridge,
    ChangeCartridge,
    CreateMemoryCard,
    UnloadMemoryCard,
    LoadMemoryCard,
    ChangeMemoryCard
};

DelayedFileActions PendingAction = DelayedFileActions::None;
string PendingActionPath = "";


// =============================================================================
//      INDIVIDUAL MENUS IN THE MENU BAR
// =============================================================================


void ProcessMenuConsole()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Console) ) )
      return;
    
    if( Vircon.PowerIsOn )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Console_PowerOff) ) )
          Vircon.PowerOff();
          
        if( ImGui::MenuItem( Texts(TextIDs::Console_Reset) ) )
        {
            Vircon.Reset();
            ShowEmulatorWindow();
        }
    }
    else
    {
        if( ImGui::MenuItem( Texts(TextIDs::Console_PowerOn) ) )
        {
            Vircon.PowerOn();
            MouseIsOnWindow = false;
        }
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Console_Quit) ) )
      GlobalLoopActive = false;
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuCartridge()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Cartridge) ) )
      return;
    
    // first, display the current cartridge
    if( !Vircon.HasCartridge() )
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text( Texts(TextIDs::Cartridge_NoCartridge) );
        ImGui::PopStyleVar();
        ImGui::Separator();
    }
    else
    {
        string DisplayedName = "[ " + Vircon.CartridgeController.CartridgeFileName + " ]";
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text( DisplayedName.c_str() );
        ImGui::PopStyleVar();
        ImGui::Separator();
    }
    
    // now display the actual options
    if( !Vircon.PowerIsOn )
    {
        if( !Vircon.HasCartridge() )
        {
            if( ImGui::MenuItem( Texts(TextIDs::Cartridge_Load) ) )
              PendingAction = DelayedFileActions::LoadCartridge;
        }
        
        else
        {
            if( ImGui::MenuItem( Texts(TextIDs::Cartridge_Change) ) )
              PendingAction = DelayedFileActions::ChangeCartridge;
            
            if( ImGui::MenuItem( Texts(TextIDs::Cartridge_Unload) ) )
              PendingAction = DelayedFileActions::UnloadCartridge;
        }
    }
    
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text( Texts(TextIDs::Cartridge_Locked) );
        ImGui::PopStyleVar();
    }
    
    if( !Vircon.PowerIsOn )
    {
        // show title for recent files list
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        {
            ImGui::Separator();
            ImGui::Text( Texts(TextIDs::Cartridge_RecentTitle) );
        
            if( RecentCartridgePaths.empty() )
              ImGui::Text( Texts(TextIDs::Cartridge_RecentEmpty) );
        }
        ImGui::PopStyleVar();
        
        // show the list of last files
        int OrderNumber = 1;
        
        for( string& CartridgePath: RecentCartridgePaths )
        {
            string FileName = GetPathFileName( CartridgePath );
            FileName = to_string( OrderNumber++ ) + ") " + FileName;
            
            if( ImGui::MenuItem( FileName.c_str() ) )
            {
                PendingAction = DelayedFileActions::ChangeCartridge;
                PendingActionPath = CartridgePath;
            }
        }
        
        // add the option to clear the list
        if( !RecentCartridgePaths.empty() )
          if( ImGui::MenuItem( Texts(TextIDs::Cartridge_RecentClear) ) )
            RecentCartridgePaths.clear();
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuMemoryCard()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Card) ) )
      return;
    
    // first, display the current cartridge
    if( !Vircon.HasMemoryCard() )
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text( Texts(TextIDs::Card_NoCard) );
        ImGui::PopStyleVar();
        ImGui::Separator();
    }
    else
    {
        string DisplayedName = "[ " + Vircon.MemoryCardController.CardFileName + " ]";
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text( DisplayedName.c_str() );
        ImGui::PopStyleVar();
        ImGui::Separator();
    }
    
    // now display the actual options
    if( ImGui::MenuItem( Texts(TextIDs::Card_Create) ) )
      PendingAction = DelayedFileActions::CreateMemoryCard;
    
    if( !Vircon.HasMemoryCard() )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Card_Load) ) )
          PendingAction = DelayedFileActions::LoadMemoryCard;
    }
    
    else
    {
        if( ImGui::MenuItem( Texts(TextIDs::Card_Change) ) )
          PendingAction = DelayedFileActions::ChangeMemoryCard;
        
        if( ImGui::MenuItem( Texts(TextIDs::Card_Unload) ) )
          PendingAction = DelayedFileActions::UnloadMemoryCard;
    }
    
    // show title for recent files list
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    {
        ImGui::Separator();
        ImGui::Text( Texts(TextIDs::Card_RecentTitle) );
    
        if( RecentCartridgePaths.empty() )
          ImGui::Text( Texts(TextIDs::Card_RecentEmpty) );
    }
    ImGui::PopStyleVar();
    
    // show the list of last files
    int OrderNumber = 1;
    
    for( string& MemoryCardPath: RecentMemoryCardPaths )
    {
        string FileName = GetPathFileName( MemoryCardPath );
        FileName = to_string( OrderNumber++ ) + ") " + FileName;
        
        if( ImGui::MenuItem( FileName.c_str() ) )
        {
            PendingAction = DelayedFileActions::ChangeMemoryCard;
            PendingActionPath = MemoryCardPath;
        }
    }
    
    // add the option to clear the list
    if( !RecentMemoryCardPaths.empty() )
      if( ImGui::MenuItem( Texts(TextIDs::Card_RecentClear) ) )
        RecentMemoryCardPaths.clear();
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuGamepads()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Gamepads) ) )
      return;
    
    // check if the keyboard is free
    bool KeyboardIsUsed = false;
    
    for( int Gamepad = 0; Gamepad < 4; Gamepad++ )
      if( MappedGamepads[ Gamepad ].Type == DeviceTypes::Keyboard )
        KeyboardIsUsed = true;
    
    // show devices for the 4 gamepads
    for( int Gamepad = 0; Gamepad < 4; Gamepad++ )
    {
        string MenuText = Texts(TextIDs::Gamepads_Gamepad) + string(" ") + to_string( Gamepad+1 );
        bool OptionSelected;
        
        if( ImGui::BeginMenu( MenuText.c_str() ) )
        {
            OptionSelected = (MappedGamepads[ Gamepad ].Type == DeviceTypes::NoDevice);
            
            if( ImGui::MenuItem( Texts(TextIDs::Gamepads_NoDevice), nullptr, OptionSelected, true ) )
            {
                MappedGamepads[ Gamepad ].Type = DeviceTypes::NoDevice;
                AssignInputDevices();
            }
            
            OptionSelected = (MappedGamepads[ Gamepad ].Type == DeviceTypes::Keyboard);
            
            bool DisableKeyboard = KeyboardIsUsed && !OptionSelected;
            
            if( DisableKeyboard )
              ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            
            // check if the keyboard is free
            if( ImGui::MenuItem( Texts(TextIDs::Gamepads_Keyboard), nullptr, OptionSelected, !DisableKeyboard ) )
            {
                MappedGamepads[ Gamepad ].Type = DeviceTypes::Keyboard;
                AssignInputDevices();
            }
            
            if( KeyboardIsUsed && MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
              ImGui::PopStyleVar();
            
            // show every available profile
            for( auto Pair = JoystickProfiles.begin(); Pair != JoystickProfiles.end(); Pair++ )
            {
                JoystickMapping* JoystickProfile = Pair->second;
                OptionSelected = (MappedGamepads[ Gamepad ].Type == DeviceTypes::Joystick)
                              && (MappedGamepads[ Gamepad ].GUID == JoystickProfile->GUID);
                
                if( ImGui::MenuItem( JoystickProfile->ProfileName.c_str(), nullptr, OptionSelected, true ) )
                {
                    MappedGamepads[ Gamepad ].Type = DeviceTypes::Joystick;
                    MappedGamepads[ Gamepad ].GUID = JoystickProfile->GUID;
                    AssignInputDevices();
                }
            }
            
            ImGui::EndMenu();
        }
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuOptions()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Options) ) )
      return;
    
    if( ImGui::BeginMenu( Texts(TextIDs::Options_VideoSize) ) )
    {
        if( ImGui::MenuItem( "x1  (Ctrl+1)" ) )
          SetWindowZoom( 1 );
          
        if( ImGui::MenuItem( "x2  (Ctrl+2)" ) )
          SetWindowZoom( 2 );
        
        if( ImGui::MenuItem( "x3  (Ctrl+3)" ) )
          SetWindowZoom( 3 );
        
        if( ImGui::MenuItem( Texts(TextIDs::Options_FullScreen) ) )
          SetFullScreen();
        
        ImGui::EndMenu();
    }
    
    if( ImGui::BeginMenu( Texts(TextIDs::Options_SoundVolume) ) )
    {
        // process volume slider
        int Volume = 100 * Vircon.GetOutputVolume();
        ImGui::SetNextItemWidth( 80 * OpenGL2D.WindowWidth / Constants::ScreenWidth );
        
        if( ImGui::SliderInt( "##Volume", &Volume, 0, 100 ) )
          Vircon.SetOutputVolume( Volume / 100.0 );
        
        // process mute checkbox
        bool Mute = Vircon.IsMuted();
        
        if( ImGui::Checkbox( Texts(TextIDs::Options_Mute), &Mute ) )
          Vircon.SetMute( Mute );
        
        ImGui::EndMenu();
    }
    
    if( ImGui::BeginMenu( Texts(TextIDs::Options_Language) ) )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Options_English), nullptr, (CurrentLanguage == &LanguageEnglish[0]), true ) )
          SetLanguage( "English" );
          
        if( ImGui::MenuItem( Texts(TextIDs::Options_Spanish), nullptr, (CurrentLanguage == &LanguageSpanish[0]), true ) )
          SetLanguage( "Spanish" );
        
        ImGui::EndMenu();
    }
    
    // allow to take a screenshot only when console is turned on
    if( ImGui::MenuItem( Texts(TextIDs::Options_Screenshot), nullptr, false, Vircon.PowerIsOn ) )
      GUI_SaveScreenshot();
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuHelp()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Help) ) )
      return;
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_QuickGuide) ) )
    {
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts(TextIDs::Dialogs_Guide_Title),
            Texts(TextIDs::Dialogs_Guide_Label)
        );
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_ShowReadme) ) )
    {
        // unfortunately the way to open a text file is
        // dependent on the underlying operating system
        #if defined(__linux__)
          string ReadmePath = "xdg-open \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
        
        #elif defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
          string ReadmePath = "start notepad \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
        
        #elif defined(__APPLE__)
          string ReadmePath = "open -a TextEdit \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
        
        #else
          #error No information on how to show Readme file in this operating system!

        #endif
        
        system( ReadmePath.c_str() );
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_About) ) )
    {
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts(TextIDs::Dialogs_About_Title),
            Texts(TextIDs::Dialogs_About_Label)
        );
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessLabelCPU()
{
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    
    // loads are not applicable if the machine is off
    if( !Vircon.PowerIsOn )
      ImGui::Text( Texts(TextIDs::Status_ConsoleOff) );
    
    // not applicable either if the machine is halted
    else if( Vircon.CPU.Halted )
      ImGui::Text( Texts(TextIDs::Status_CPUHalted) );
    
    // show the maximum load of the last 2 frames
    else
    {
        int CPULoad = max( Vircon.LastCPULoads[0], Vircon.LastCPULoads[1] );
        int GPULoad = max( Vircon.LastGPULoads[0], Vircon.LastGPULoads[1] );
        ImGui::Text( "CPU %d%%, GPU %d%%", CPULoad, GPULoad );
    }
    
    ImGui::PopStyleVar();
}


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


// since GUI overlaps the emulator output, we need some
// sensible policy to decide when it is actually needed
bool GUIMustBeDrawn()
{
    if( OpenGL2D.FullScreen )
      return ImGui::GetIO().WantCaptureMouse;
    
    return MouseIsOnWindow;
}

// -----------------------------------------------------------------------------

// renders the emulator's framebuffer onto the main program's
// window to make it visible. Since the console implementation
// can change OpenGL's render properties, we need to wrap this
// to ensure the framebuffer is rendered correctly
void ShowEmulatorWindow()
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    
    // to do the actual drawing on the screen
    // correctly we have to temporarily 
    // override render settings in OpenGL
    OpenGL2D.SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    OpenGL2D.SetTranslation( 0, 0 );
    OpenGL2D.ComposeTransform( false, false );
    OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // if the emulator is on, draw its display
    // on our window; otherwise just show a "no
    // signal" indicator on a black screen
    OpenGL2D.RenderToScreen();
    
    if( Vircon.PowerIsOn )
      OpenGL2D.DrawFramebufferOnScreen();
    else
      NoSignalTexture.Draw( OpenGL2D, 0, 0, Constants::ScreenWidth, Constants::ScreenHeight );
    
    // if GUI is showing, darken the screen
    if( GUIMustBeDrawn() )
      OpenGL2D.ClearScreen( GPUColor{ 0, 16, 32, 210 } );
    
    // now restore the Vircon render parameters
    VirconWord BlendValue;
    BlendValue.AsInteger = Vircon.GPU.ActiveBlending;
    Vircon.GPU.WritePort( (int32_t)GPU_LocalPorts::ActiveBlending, BlendValue );
    OpenGL2D.MultiplyColor = Vircon.GPU.MultiplyColor;
}

// -----------------------------------------------------------------------------

// handles all calls to ImGui to manage and render all of our
// GUI elements onto the main program's window. Since the console
// implementation can change OpenGL's render properties, we need
// to wrap this to ensure the framebuffer is rendered correctly
void RenderGUI()
{
    // delete any previous pending actions
    PendingAction = DelayedFileActions::None;
    PendingActionPath = "";
    
    // remove any emulator blending modes
    OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // start new frame in imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame( OpenGL2D.Window );
    ImGui::NewFrame();
    
    // show the main menu bar
    if( ImGui::BeginMainMenuBar() )
    {
        // menus
        ProcessMenuConsole();
        ProcessMenuCartridge();
        ProcessMenuMemoryCard();
        ProcessMenuGamepads();
        ProcessMenuOptions();
        ProcessMenuHelp();
        
        // CPU% label
        ProcessLabelCPU();
        
        ImGui::EndMainMenuBar();
    }
    
    // (2) Render imgui
    if( GUIMustBeDrawn() )
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    }
    else ImGui::EndFrame();
    
    // pause the emulator when GUI is used
    if( GUIMustBeDrawn() )
      Vircon.Pause();
    
    else Vircon.Resume();
    
    // restore emulator blending modes
    VirconWord BlendValue;
    BlendValue.AsInteger = Vircon.GPU.ActiveBlending;
    Vircon.GPU.WritePort( (int32_t)GPU_LocalPorts::ActiveBlending, BlendValue );
    
    // only after GUI processing is done, we can
    // safely perform any file processing actions
    // that were requested from menu options
    switch( PendingAction )
    {
        // cartridge file actions
        case DelayedFileActions::UnloadCartridge:
            GUI_UnloadCartridge();
            break;
        case DelayedFileActions::LoadCartridge:
            GUI_LoadCartridge( PendingActionPath );
            break;
        case DelayedFileActions::ChangeCartridge:
            GUI_ChangeCartridge( PendingActionPath );
            break;
            
        // memory card file actions
        case DelayedFileActions::CreateMemoryCard:
            GUI_CreateMemoryCard();
            break;
        case DelayedFileActions::UnloadMemoryCard:
            GUI_UnloadMemoryCard();
            break;
        case DelayedFileActions::LoadMemoryCard:
            GUI_LoadMemoryCard( PendingActionPath );
            break;
        case DelayedFileActions::ChangeMemoryCard:
            GUI_ChangeMemoryCard( PendingActionPath );
            break;
        
        // in other cases no actions are performed
        case DelayedFileActions::None: break;
        default: break;
    }
}
