// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    
    // include project headers
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


// debug configuration (for the assembler itself)
bool DebugMode = false;
bool VerboseMode = false;

// assembler configuration (for the generated binary)
string AssemblerFolder;
int InitialROMAddress = Constants::CartridgeProgramROMFirstAddress;
bool CreateDebugVersion = false;