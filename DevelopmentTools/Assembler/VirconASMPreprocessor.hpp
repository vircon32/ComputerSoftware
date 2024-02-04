// *****************************************************************************
    // start include guard
    #ifndef VIRCONASMPREPROCESSOR_HPP
    #define VIRCONASMPREPROCESSOR_HPP
    
    // include project headers
    #include "Tokens.hpp"
    #include "VirconASMLexer.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


/* ------------------------------- GENERAL NOTES--------------------------------

  1) Since we need to iterate on processing contexts, instead of using the
     more conceptually accurate std::stack we need to use other containers.

  2) Processing context class is not ready to be copy-constructed as it is.
     Since std::vector may reallocate on expansion, and this needs to copy-
     construct, we will instead prefer std::list.
----------------------------------------------------------------------------- */


// =============================================================================
//      CONTEXT CLASSES FOR THE PREPROCESSOR
// =============================================================================


typedef struct
{
    int StartingLine;
    bool ConditionIsMet;
    bool ElseWasFound;
}
IfContext;

// -----------------------------------------------------------------------------

class ProcessingContext
{
    public:
        
        std::string FilePath;
        std::string ReferenceFolder;
        
        // parsing progress within lexer lines
        std::list< TokenList > SourceLines;   // copies so that nested lexers can be destroyed
        std::list< TokenList >::iterator LinePosition;
        
        // nested "if" contexts
        std::list< IfContext > IfStack;
        
    public:
        
        // instance handling
        ProcessingContext();
       ~ProcessingContext();
        
        void Advance();
        bool LinesHaveEnded();
        TokenList& GetCurrentLine();
        bool AreAllIfConditionsMet();
};


// =============================================================================
//      VIRCON ASM PREPROCESSOR
// =============================================================================


class VirconASMPreprocessor
{
    public: // protected:
        
        std::list< ProcessingContext > ContextStack;
        std::map< std::string, TokenList > Definitions;
        
    public:
        
        // resulting tokens (all tokens are actually
        // a copy to account for modified locations)
        TokenList ProcessedTokens;
        
    protected:
        
        // error handling
        void EmitError( SourceLocation Location, const std::string& Description, bool Abort = true );
        void EmitWarning( SourceLocation Location, const std::string& Description );
        
        // context handling
        void PushContext( VirconASMLexer& Lexer );
        void PushContext( SourceLocation Location, const std::string& FilePath );
        void PopContext();
        
        // insertion functions
        bool ReplaceDefinitions( TokenList& Line );
        void IncludeFile( SourceLocation Location, const std::string& FilePath );
        
        // helper processor functions
        std::string ExpectIdentifier( TokenIterator& TokenPosition );
        void ProcessLine();
        
        // processor functions for specific directives
        // (they return the new start of the next line)
        void ProcessError( bool WarningOnly );
        void ProcessIf( bool IsIfndef );
        void ProcessInclude();
        void ProcessDefine();
        void ProcessUndef();
        void ProcessElse();
        void ProcessEndif();
        
    public:
        
        // instance handling
       ~VirconASMPreprocessor();
        
        // main processing function
        void Preprocess( VirconASMLexer& Lexer );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
