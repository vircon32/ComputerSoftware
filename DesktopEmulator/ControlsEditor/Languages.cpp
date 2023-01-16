// *****************************************************************************
    // include infrastructure headers
    #include "../DesktopInfrastructure/LogStream.hpp"
    
    // include project headers
    #include "Languages.hpp"
    #include "Globals.hpp"
    
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
    "EditControls version 23.1.16."
    "\n"
    "\nThis program is a tool included with the Vircon32 emulator."
    "\nIt can create control profiles for keyboard and joysticks."
    "\n\n"
    "\nEditControls and Vircon32 were created by Carra.";
;
 
string GuideTextEnglish =
    "How to use this program:"
    "\n"
    "\nTHE KEYBOARD:"
    "\n------------------------------------------"
    "\n1) The keyboard profile always exists and can be edited."
    "\n2) Click on each Vircon32 gamepad element and press your chosen key."
    "\n"
    "\nJOYSTICKS:"
    "\n------------------------------------------"
    "\n1) Connect any joysticks that you want to configure."
    "\n2) Find them in the devices menu and create a profile for them."
    "\n3) Change the profile name to one you will remember."
    "\n4) Click on each Vircon32 gamepad element and press a joystick control."
    "\n"
    "\nAFTER THAT:"
    "\n------------------------------------------"
    "\n1) Exit and save the changes. Your devices are now saved to an XML file."
    "\n2) Open the emulator and go to the 'Gamepads' menu."
    "\n3) For each of the 4 gamepads, assign the device you want to use."
    "\n4) The keyboard can only be assigned to one gamepad."
    "\n5) If you have N identical joysticks, assign that profile to N gamepads.";
;

string LanguageEnglish[] = 
{
    "",
    "File ",
    "Connected devices ",
    "Profiles ",
    "Language ",
    "Help ",
    "Set factory defaults",
    "Reload controls file",
    "Exit (saving changes)",
    "Exit (without saving)",
    "Keyboard",
    "Create new profile",
    "Edit profile",
    "Delete profile",
    "Keyboard",
    "Edit profile",
    "Delete profile",
    "English",
    "Spanish",
    "Quick guide",
    "About",
    "Profile name:",
    "Device name:",
    "Device GUID:",
    "Keyboard",
    "(none)",
    
    "Waiting for key...",
    "Press a new key for this control",
    "(or Escape key to cancel)",
    "Clear key",
    "Waiting for joystick...",
    "Press a new joystick button/direction for this control",
    "(or Escape key to cancel)",
    "Clear control",
    
    "OK",
    "Cancel",
    "Yes",
    "No",
    "Are you sure?",
    "Replace current profiles with the defaults?",
    "Load from file and replace all profiles?",
    "Exit and save all changes?",
    "Exit without saving changes?",
    "Delete this profile?",
    "About EditControls",
    AboutTextEnglish,
    "Quick guide",
    GuideTextEnglish,
    
    "EditControls had an error",
    "EditControls failed.\nReason: ",
    "Cannot load controls file.\nReason: ",
    "Setting default controls.",
    "Cannot save controls file.\nReason: "
};


// =============================================================================
//      LANGUAGE: SPANISH
// =============================================================================


string AboutTextSpanish =
    "EditControls versi\u00F3n 23.1.16."
    "\n"
    "\nEste programa es una herramienta incluida con el emulador de Vircon32."
    "\nPermite crear perfiles de control para teclado y joysticks."
    "\n\n"
    "\nEditControls y Vircon32 han sido creados por Carra.";
;

string GuideTextSpanish =
    "C\u00F3mo usar este programa:"
    "\n"
    "\nEL TECLADO:"
    "\n------------------------------------------"
    "\n1) El perfil del teclado siempre existe y se puede editar."
    "\n2) Haz click en cada control del mando de Vircon32 y pulsa la tecla que quieras."
    "\n"
    "\nJOYSTICKS:"
    "\n------------------------------------------"
    "\n1) Conecta todos los joysticks que quieras configurar."
    "\n2) B\u00FAscalos en el men\u00FA de aparatos y crea un perfil para ellos."
    "\n3) Cambia el nombre del perfil a uno que puedas recordar."
    "\n4) Haz click en cada control del mando de Vircon32 y pulsa un control del joystick."
    "\n"
    "\nDESPU\u00C9S DE ESO:"
    "\n------------------------------------------"
    "\n1) Sal y guardando los cambios. Ahora tus aparatos se han guardado en un archivo XML."
    "\n2) Abre el emulador y ve al men\u00FA 'Mandos'."
    "\n3) En cada uno de los 4 mandos, asigna el aparato que quieras usar."
    "\n4) El teclado s\u00F3lo se puede asignar a un mando."
    "\n5) Si tienes N joysticks id\u00E9nticos, asigna ese perfil a N mandos.";
;

string LanguageSpanish[] = 
{
    "",
    "Archivo ",
    "Aparatos conectados ",
    "Perfiles ",
    "Idioma ",
    "Ayuda ",
    "Volver a ajustes de f\u00E1brica",
    "Recargar archivo de controles",
    "Salir (guardando cambios)",
    "Salir (sin guardar)",
    "Teclado",
    "Crear nuevo perfil",
    "Editar perfil",
    "Borrar perfil",
    "Teclado",
    "Editar perfil",
    "Borrar perfil",
    "Ingl\u00E9s",
    "Espa\u00F1ol",
    "Gu\u00EDa r\u00E1pida",
    "Acerca de",
    "Nombre perfil:",
    "Nombre aparato:",
    "GUID aparato:",
    "Teclado",
    "(ninguno)",
    
    "Esperando tecla...",
    "Pulsa nueva tecla para este control",
    "(o tecla Escape para cancelar)",
    "Borrar tecla",
    "Esperando al joystick...",
    "Pulsa nuevo bot\u00F3n o direcci\u00F3n del joystick para este control",
    "(o tecla Escape para cancelar)",
    "Borrar control",
    
    "Aceptar",
    "Cancelar",
    "S\u00ED",
    "No",
    "\u00BFSeguro que quieres hacer esto?",
    "\u00BFReemplazar los perfiles actuales con los que hay por defecto?",
    "\u00BFCargar del archivo y reemplazar todos los perfiles?",
    "\u00BFSalir guardando todos los cambios?",
    "\u00BFSalir sin guardar cambios?",
    "\u00BFBorrar este perfil?",
    "Acerca de EditControls",
    AboutTextSpanish,
    "Gu\u00EDa r\u00E1pida",
    GuideTextSpanish,
    
    "EditControls tuvo un error",
    "EditControls ha fallado.\nCausa: ",
    "No se puede cargar archivo de controles.\nCausa: ",
    "Aplicando los controles por defecto.",
    "No se puede guardar archivo de controles.\nCausa: "
};
