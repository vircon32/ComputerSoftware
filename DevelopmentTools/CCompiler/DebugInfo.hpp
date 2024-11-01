// *****************************************************************************
    // include project headers
    #include "VirconCPreprocessor.hpp"
    #include "VirconCParser.hpp"
    #include "VirconCEmitter.hpp"
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO OUTPUT FILES WITH DEBUG INFORMATION
// =============================================================================


// save debug info for the program
void SaveDebugInfoFile( const std::string& FilePath, const VirconCParser& Parser, const VirconCEmitter& Emitter );

// save debug logs for the internal stages of the compiler itself
void SaveLexerLog( const std::string& FilePath, const VirconCPreprocessor& Preprocessor );
void SaveParserLog( const std::string& FilePath, const VirconCParser& Parser );
