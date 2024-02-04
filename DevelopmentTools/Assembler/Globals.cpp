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


// debug confiugration
bool Debug = false;
bool VerboseMode = false;

// assembler configuration
string AssemblerFolder;
int InitialROMAddress = Constants::CartridgeProgramROMFirstAddress;
