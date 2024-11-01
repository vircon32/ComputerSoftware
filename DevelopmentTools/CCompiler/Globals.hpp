// *****************************************************************************
    // start include guard
    #ifndef GLOBALS_HPP
    #define GLOBALS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      GLOBAL CONFIGURATION
// =============================================================================


extern std::string CompilerFolder;
extern bool CompileOnly;
extern bool DisableWarnings;
extern bool EnableAllWarnings;


// =============================================================================
//      DEBUG
// =============================================================================


// debug configuration for the compiler itself
extern bool DebugMode;
extern bool VerboseMode;

// debug configuration for the generated binary
extern bool CreateDebugVersion;



// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

