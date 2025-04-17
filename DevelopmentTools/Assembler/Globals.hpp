// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


// debug configuration (for the assembler itself)
extern bool DebugMode;
extern bool VerboseMode;

// assembler configuration (for the generated binary)
extern std::string AssemblerFolder;
extern int InitialROMAddress;
extern bool CreateDebugVersion;
extern bool CartridgeOffset;
extern bool VBINOffset;


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
