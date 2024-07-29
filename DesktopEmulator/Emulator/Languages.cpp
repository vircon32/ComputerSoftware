// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "Languages.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      LANGUAGE SELECTION
// =============================================================================


map< string, string* > Languages;
string* CurrentLanguage = LanguageEnglish;

// -----------------------------------------------------------------------------

void SetLanguage( string LanguageName )
{
    CurrentLanguage = Languages[ LanguageName ];
}

// -----------------------------------------------------------------------------

const char* Texts( TextIDs ID )
{
    return CurrentLanguage[ (int)ID ].c_str();
}


// =============================================================================
//      LANGUAGE: ENGLISH
// =============================================================================


string AboutTextEnglish =
    "Vircon32 Emulator version 24.7.29."
    "\n"
    "\nEmulator for the Vircon32 virtual console."
    "\nThis program allows you to play Vircon32 games."
    "\n\n"
    "\nThis emulator and Vircon32 were created by Carra"
;

string GuideTextEnglish =
    "How to use this emulator:"
    "\n"
    "\nPLAYING GAMES:"
    "\n------------------------------------------"
    "\n  First use the Cartridge menu to load a Vircon32 rom (*.v32 files)."
    "\n  Then, at Console menu power on the console and the game will start."
    "\n  To prevent errors, the cartridge slot gets blocked when power is on."
    "\n"
    "\nDEFAULT CONTROLS:"
    "\n------------------------------------------"
    "\n  By default only gamepad 1 is configured, and it uses the keyboard."
    "\n  Arrow keys control the D-Pad directions, and buttons use:"
    "\n   - Buttons L, R --> Keys Q, W"
    "\n   - Buttons Y, X --> Keys A, S"
    "\n   - Buttons B, A --> Keys Z, X"
    "\n   - Button Start --> Key Return"
    "\n"
    "\nCHANGING CONTROLS:"
    "\n------------------------------------------"
    "\n  You can change the default keyboard controls and configure joysticks."
    "\n  To do this, close the emulator and use the program called EditControls"
    "\n  that should be in this same folder. After your control profiles are"
    "\n  saved you can come back to the emulator and use the Gamepads menu to"
    "\n  assign your profiles to each of the 4 console gamepads."
    "\n\n"
    "If you need more help, you can click on 'Show readme file'."
;

string LanguageEnglish[] = 
{
    "",
    "Console ",
    "Cartridge ",
    "Memory Card ",
    "Gamepads ",
    "Options ",
    "Help ",
    "Power ON  (Ctrl+P)",
    "Power OFF  (Ctrl+P)",
    "Reset  (Ctrl+R)",
    "Quit  (Ctrl+Q)",
    "No cartridge loaded",
    "(Cartridge slot is locked)",
    "Load cartridge...  (Ctrl+L)",
    "Unload cartridge  (Ctrl+U)",
    "Change cartridge...  (Ctrl+L)",
    "Recent cartridges:",
    "(Empty)",
    "Clear list",
    "No card loaded",
    "Create empty card...",
    "Load memory card...",
    "Unload memory card",
    "Change memory card...",
    "Recent memory cards:",
    "(Empty)",
    "Clear list",
    "Gamepad",
    "No device",
    "Keyboard",
    "Video size",
    "Sound volume",
    "Memory card handling",
    "Language",
    "Save screenshot  (Ctrl+S)",
    "Full screen  (Ctrl+F)",
    "Mute  (Ctrl+M)",
    "Automatic (one for each game)",
    "Manual (use card menu)",
    "English",
    "Spanish",
    "Quick guide",
    "Show Readme file",
    "About",
    "(CONSOLE OFF)",
    "(CPU HALTED)",
    "No cartridge",
    
    "OK",
    "Cancel",
    "Yes",
    "No",
    "Done",
    "Memory card is created",
    "Screenshot is saved",
    "About Vircon32 Emulator",
    AboutTextEnglish,
    "Quick guide",
    GuideTextEnglish,
    "Readme.txt",
    
    "The emulator had an error",
    "Vircon32 emulator failed.\nReason: ",
    "Cannot create a memory card.\nReason: ",
    "Cannot load memory card.\nReason: ",
    "Cannot unload memory card.\nReason: ",
    "Cannot change memory card.\nReason: ",
    "Cannot auto-update memory card.\nReason: ",
    "Cannot load cartridge.\nReason: ",
    "Cannot unload cartridge.\nReason: ",
    "Cannot change cartridge.\nReason: ",
    "Cannot save screenshot.\nReason: ",
    "Cannot load controls file.\nReason: ",
    "Setting default controls.",
    "Invalid device",
    "Could not find device named ",
    "Gamepad ",
    " will be set to 'No device'",
    "Cannot load settings file.\nReason: ",
    "Applying default settings",
    "Cannot save settings file.\nReason: ",
    "Invalid language",
    "Settings file contains an invalid language.\nEnglish language will be used"
};


// =============================================================================
//      LANGUAGE: SPANISH
// =============================================================================


string AboutTextSpanish =
    "Emulador de Vircon32 versi\u00F3n 24.7.29."
    "\n"
    "\nEmulador de la consola virtual Vircon32."
    "\nEste programa te permite jugar a juegos de Vircon32."
    "\n\n"
    "\nEste emulador y Vircon32 han sido creados por Carra"
;

string GuideTextSpanish =
    "C\u00F3mo usar este emulador:"
    "\n"
    "\nJUGAR A JUEGOS:"
    "\n------------------------------------------"
    "\n  Primero usa el men\u00FA Cartucho para cargar una rom de Vircon32 rom"
    "\n  (archivos *.v32). Luego, en el men\u00FA Consola enciende la consola y"
    "\n  el juego empezar\u00E1. Para evitar errores la ranura de cartucho se"
    "\n  bloquea cuando la consola est\u00E1 encendida."
    "\n"
    "\nCONTROLES POR DEFECTO:"
    "\n------------------------------------------"
    "\n  Por defecto s\u00F3lo el mando 1 est\u00E1 configurado, y usa el teclado. Las"
    "\n  teclas de flecha controlan las direcciones (la cruceta). Los botones usan:"
    "\n   - Botones L, R --> Teclas Q, W"
    "\n   - Botones Y, X --> Teclas A, S"
    "\n   - Botones B, A --> Teclas Z, X"
    "\n   - Bot\u00F3n Start --> Tecla Return"
    "\n"
    "\nCAMBIAR LOS CONTROLES:"
    "\n------------------------------------------"
    "\n  Puedes cambiar los controles por defecto del teclado y configurar"
    "\n  joysticks. Para hacerlo, cierra el emulador y usa el programa"
    "\n  llamado EditControls que deber\u00EDa estar en esta misma carpeta. Cuando"
    "\n  tus perfiles de control se hayan guardado puedes volver al emulador"
    "\n  y usar el men\u00FA Mandos para asignar esos perfiles a cada uno de los 4"
    "\n  mandos de la consola."
    "\n\n"
    "Si necesitas m\u00E1s ayuda, puedes hacer click en 'Ver archivo Readme'."
;

string LanguageSpanish[] = 
{
    "",
    "Consola ",
    "Cartucho ",
    "Tarj. memoria ",
    "Mandos ",
    "Opciones ",
    "Ayuda ",
    "Encender  (Ctrl+P)",
    "Apagar  (Ctrl+P)",
    "Reiniciar  (Ctrl+R)",
    "Salir  (Ctrl+Q)",
    "Ning\u00FAn cartucho cargado",
    "(Ranura de cartucho bloqueada)",
    "Cargar cartucho...  (Ctrl+L)",
    "Quitar cartucho  (Ctrl+U)",
    "Cambiar cartucho...  (Ctrl+L)",
    "Cartuchos recientes:",
    "(Vac\u00EDo)",
    "Limpiar lista",
    "Ninguna tarjeta cargada",
    "Crear tarjeta vac\u00EDa...",
    "Cargar tarjeta...",
    "Quitar tarjeta",
    "Cambiar tarjeta...",
    "Tarjetas recientes:",
    "(Vac\u00EDo)",
    "Limpiar lista",
    "Mando",
    "Ning\u00FAn dispositivo",
    "Teclado",
    "Tama\u00F1o de video",
    "Volumen del sonido",
    "Gesti\u00F3n de tarjetas",
    "Idioma",
    "Capturar pantalla  (Ctrl+S)",
    "Pantalla completa  (Ctrl+F)",
    "Silenciar  (Ctrl+M)",
    "Autom\u00E1tica (una por cada juego)",
    "Manual (usar men\u00FA)",
    "Ingl\u00E9s",
    "Espa\u00F1ol",
    "Gu\u00EDa r\u00E1pida",
    "Ver archivo Readme",
    "Acerca de",
    "(APAGADO)",
    "(CPU PARADA)",
    "Sin cartucho",
    
    "Aceptar",
    "Cancelar",
    "S\u00ED",
    "No",
    "Hecho",
    "Se ha creado la tarjeta de memoria",
    "La captura de pantalla se ha guardado",
    "Sobre el emulador de Vircon32",
    AboutTextSpanish,
    "Gu\u00EDa r\u00E1pida",
    GuideTextSpanish,
    "Readme (Spanish).txt",
    
    "El emulador ha tenido un error",
    "El emulador de Vircon32 ha fallado.\nCausa: ",
    "No se puede crear la tarjeta de memoria.\nCausa: ",
    "No se puede cargar la tarjeta de memoria.\nCausa: ",
    "No se puede quitar la tarjeta de memoria.\nCausa: ",
    "No se puede cambiar la tarjeta de memoria.\nCausa: ",
    "No se puede auto-actualizar la tarjeta de memoria.\nCausa: ",
    "No se puede cargar el cartucho.\nCausa: ",
    "No se puede quitar el cartucho.\nCausa: ",
    "No se puede cambiar el cartucho.\nCausa: ",
    "No se puede guardar la captura de pantalla.\nCausa: ",
    "No se puede cargar el archivo de controles.\nCausa: ",
    "Aplicando los controles por defecto.",
    "Dispositivo no v\u00E1lido",
    "No se puede encontrar el dispositivo con nombre ",
    "El mando ",
    " se asignar\u00E1 a 'Ning\u00FAn dispositivo'",
    "No se puede cargar el archivo de opciones.\nCausa: ",
    "Aplicando las opciones por defecto",
    "No se puede guardar el archivo de opciones.\nCausa: ",
    "Idioma no v\u00E1lido",
    "El archivo de opciones contiene un idioma no v\u00E1lido.\nSe usar\u00E1 el idioma ingl\u00E9s"
};
