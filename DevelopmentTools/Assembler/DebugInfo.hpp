// *****************************************************************************
    // include project headers
    #include "VirconASMPreprocessor.hpp"
    #include "VirconASMParser.hpp"
    #include "VirconASMEmitter.hpp"
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO OUTPUT FILES WITH DEBUG INFORMATION
// =============================================================================


// save debug info for the program
void SaveDebugInfoFile( const std::string& FilePath, const VirconASMParser& Parser, const VirconASMEmitter& Emitter );

// save debug logs for the internal stages of the assembler itself
void SaveLexerLog( const std::string& FilePath, const VirconASMPreprocessor& Preprocessor );
void SaveParserLog( const std::string& FilePath, const VirconASMParser& Parser );
void SaveEmitterLog( const std::string& FilePath, const VirconASMParser& Parser );
