// *****************************************************************************
    // start include guard
    #ifndef VIRCONASMLEXER_HPP
    #define VIRCONASMLEXER_HPP
    
    // include project headers
    #include "Tokens.hpp"
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <fstream>      // [ C++ STL ] File streams
    #include <vector>       // [ C++ STL ] Vectors
// *****************************************************************************


// =============================================================================
//      CHARACTER CLASSIFICATION FUNCTIONS
// =============================================================================


bool IsAscii( char c );
bool IsPrintableAscii( char c );
bool IsInvalidAscii( char c );
bool IsWhitespace( char c );
bool IsValidNameStart( char c );
bool IsValidNameContinuation( char c );
bool IsSeparator( char c );


// =============================================================================
//      VIRCON ASM LEXER
// =============================================================================


class VirconASMLexer
{
    public:
        
        // source input file
        std::ifstream Input;
        std::string InputDirectory;
        
        // lexing state
        SourceLocation ReadLocation;
        char PreviousChar;
        
    public:
        
        // lexer results are line-based instead of
        // a whole-file list (for easier preprocessing)
        std::vector< TokenList > TokenLines;
        
    protected:
        
        // input file handling
        void OpenFile( const std::string& FilePath );
        void CloseFile();
        
        // reading from input
        char GetChar();
        char PeekChar();
        bool InputHasEnded();
        
        // error handling
        void EmitError( const std::string& Description, bool Abort = true );
        void EmitWarning( const std::string& Description );
        
        // skipping functions
        void SkipWhitespace();
        void SkipLineComment();
        void SkipUntilNextToken();
        
        // basic lexer functions
        char UnescapeCharacter( char Escaped );
        char UnescapeHexNumber();
        char ReadCharacter();
        LiteralIntegerToken* ReadHexInteger();
        Token* ReadNumber();
        std::string ReadName();
        std::string ReadString();
        
        // partial lexing functions
        Token* ReadNextToken();        
        TokenList TokenizeNextLine();
        
    public:
        
        // instance handling
        VirconASMLexer();
       ~VirconASMLexer();
        
        // full lexing function
        void TokenizeFile( const std::string& FilePath );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
