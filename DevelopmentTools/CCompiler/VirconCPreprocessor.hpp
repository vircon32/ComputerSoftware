// *****************************************************************************
    // start include guard
    #ifndef VIRCONCPREPROCESSOR_HPP
    #define VIRCONCPREPROCESSOR_HPP
    
    // include project headers
    #include "CTokens.hpp"
    #include "VirconCLexer.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    #include <vector>       // [ C++ STL ] Vectors
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
        std::list< CTokenList > SourceLines;   // copies so that nested lexers can be destroyed
        std::list< CTokenList >::iterator LinePosition;
        
        // nested "if" contexts
        std::list< IfContext > IfStack;
        
    public:
        
        // instance handling
        ProcessingContext();
       ~ProcessingContext();
        
        void Advance();
        bool LinesHaveEnded();
        CTokenList& GetCurrentLine();
        bool AreAllIfConditionsMet();
};


// =============================================================================
//      VIRCON C PREPROCESSOR
// =============================================================================


class VirconCPreprocessor
{
    public: // protected:
        
        std::list< ProcessingContext > ContextStack;
        std::map< std::string, CTokenList > Definitions;
        
    public:
        
        // resulting tokens (all tokens are actually
        // a copy to account for modified locations)
        CTokenList ProcessedTokens;
        
    protected:
        
        // context handling
        void PushContext( VirconCLexer& Lexer );
        void PushContext( SourceLocation Location, const std::string& FilePath );
        void PopContext();
        
        // insertion functions
        bool ReplaceDefinitions( CTokenList& Line );
        void IncludeFile( SourceLocation Location, const std::string& FilePath );
        
        // processor for a generic line
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
        
        // transform identifiers into keywords
        // once preprocessing is completed
        void RecognizeKeywords();
        
    public:
        
        // instance handling
       ~VirconCPreprocessor();
        
        // main processing function
        void Preprocess( VirconCLexer& Lexer );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
