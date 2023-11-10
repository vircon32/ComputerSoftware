// *****************************************************************************
    // start include guard
    #ifndef VIRCONASMLEXER_HPP
    #define VIRCONASMLEXER_HPP
    
    // include project headers
    #include "Tokens.hpp"
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <fstream>      // [ C++ STL ] File streams
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
    protected:
        
        // link to source data
        std::ifstream* Input;
        int CurrentReadLine;
        char PreviousChar;
        
    public:
        
        // results
        TokenList Tokens;
        
    protected:
        
        // reading from input
        char GetChar();
        char PeekChar();
        
        // error handling
        void EmitError( const std::string& Description, bool Abort = true );
        void EmitWarning( const std::string& Description );
        
        // skipping functions
        void SkipWhitespace();
        void SkipLineComment();
        
        // basic lexer functions
        char UnescapeCharacter( char Escaped );
        char UnescapeHexNumber();
        LiteralIntegerToken* ReadHexInteger();
        Token* ReadNumber();
        std::string ReadName();
        std::string ReadString();
        
    public:
        
        // instance handling
        VirconASMLexer();
       ~VirconASMLexer();
       
        // main lexer function
        void ReadTokens( std::ifstream& Input_ );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
