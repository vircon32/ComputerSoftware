// *****************************************************************************
    // include project headers
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GLOBAL CONFIGURATION
// =============================================================================


string CompilerFolder;
bool CompileOnly = false;
bool DisableWarnings = false;
bool EnableAllWarnings = false;


// =============================================================================
//      DEBUG
// =============================================================================


// debug configuration for the compiler itself
bool DebugMode = false;
bool VerboseMode = false;

// debug configuration for the generated binary
bool CreateDebugVersion = false;
