// *****************************************************************************
    // start include guard
    #ifndef COMPILERINFRASTRUCTURE_HPP
    #define COMPILERINFRASTRUCTURE_HPP
    
    // include project headers
    #include "SourceLocation.hpp"
    #include "CTokens.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      ESCAPING C CHARS & STRINGS
// =============================================================================


// this produces a copy, not altering the original
std::string EscapeCCharacter( char c );
std::string EscapeCString( const std::string& Text );


// =============================================================================
//      ERROR HANDLING
// =============================================================================


// global flags to abort when there were errors
extern int CompilationErrors;
extern int CompilationWarnings;

// general error functions
void RaiseWarning( SourceLocation Location, const std::string& Description );
void RaiseError( SourceLocation Location, const std::string& Description );
void RaiseFatalError( SourceLocation Location, const std::string& Description ) __attribute__((noreturn));


// =============================================================================
//      SUPPORT FUNCTIONS FOR TOKENS
// =============================================================================


void ExpectSameLine( CToken* Start, CToken* Current );
void ExpectEndOfLine( CToken* Start, CToken* Current );
void ExpectSpecialSymbol( CTokenIterator& TokenPosition, SpecialSymbolTypes Expected );
void ExpectDelimiter( CTokenIterator& TokenPosition, DelimiterTypes Expected );
void ExpectKeyword( CTokenIterator& TokenPosition, KeywordTypes Expected );
void ExpectOperator( CTokenIterator& TokenPosition, OperatorTypes Expected );
std::string ExpectIdentifier( CTokenIterator& TokenPosition );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
