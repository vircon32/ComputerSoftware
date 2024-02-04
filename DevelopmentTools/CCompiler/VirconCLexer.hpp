// *****************************************************************************
    // start include guard
    #ifndef VIRCONCLEXER_HPP
    #define VIRCONCLEXER_HPP
    
    // include project headers
    #include "CTokens.hpp"
    
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


// =============================================================================
//      VIRCON C LEXER
// =============================================================================


class VirconCLexer
{
    public:
        
        // source input file
        std::ifstream Input;
        std::string InputDirectory;
        
        // lexing state
        SourceLocation ReadLocation;
        char PreviousChar;
        bool LineIsContinued;
        
    public:
        
        // lexer results are line-based instead of
        // a whole-file list (for easier preprocessing)
        std::vector< CTokenList > TokenLines;
        
    public:
        
        // input file handling
        void OpenFile( const std::string& FilePath );
        void CloseFile();
        
        // reading from input
        char GetChar();
        char PeekChar();
        bool InputHasEnded();
        
        // skipping functions
        void SkipWhitespace();
        void SkipLineComment();
        void SkipBlockComment();
        void SkipLineAfterStray();
        void SkipUntilNextToken();
        
        // basic lexer functions
        char UnescapeCharacter( char Escaped );
        char UnescapeHexNumber();
        LiteralValueToken* ReadHexInteger();
        LiteralValueToken* ReadNumber();
        LiteralValueToken* ReadCharacter();
        LiteralStringToken* ReadString();
        OperatorToken* ReadOperator();
        std::string ReadName();        
        
        // partial lexing functions
        CToken* ReadNextToken();        
        CTokenList TokenizeNextLine();
        
    public:
        
        // instance handling
        VirconCLexer();
       ~VirconCLexer();
        
        // full lexing function
        void TokenizeFile( const std::string& FilePath );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
