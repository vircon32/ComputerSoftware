// *****************************************************************************
    // include project headers
    #include "VirconASMPreprocessor.hpp"
    #include "VirconASMParser.hpp"
    #include "VirconASMEmitter.hpp"
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO OUTPUT FILES WITH DEBUG INFORMATION
// =============================================================================


// address reference modes for debug info
enum class DebugInfoModes
{
    Program,     // addresses in words, relative to program start
    VBINFile,    // addresses in bytes, relative to assembled VBIN file
    V32File      // addresses in bytes, relative to final V32 rom file
};

// save debug info for the program
void SaveDebugInfoFile
(
    const std::string& FilePath,
    const VirconASMParser& Parser,
    const VirconASMEmitter& Emitter,
    DebugInfoModes Mode
);

// save debug logs for the internal stages of the assembler itself
void SaveLexerLog( const std::string& FilePath, const VirconASMPreprocessor& Preprocessor );
void SaveParserLog( const std::string& FilePath, const VirconASMParser& Parser );
void SaveEmitterLog( const std::string& FilePath, const VirconASMParser& Parser );
