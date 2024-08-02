// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    
    // include project headers
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


// debug configuration
bool Debug = false;
bool VerboseMode = false;

// disassembler configuration
int InitialROMAddress = Constants::CartridgeProgramROMFirstAddress;
