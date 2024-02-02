// *****************************************************************************
    // start include guard
    #ifndef VIRCONASMPARSER_HPP
    #define VIRCONASMPARSER_HPP
    
    // include project headers
    #include "Tokens.hpp"
    #include "ASTNodes.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
// *****************************************************************************


// =============================================================================
//      TRAVERSING OF TOKEN LISTS
// =============================================================================


TokenIterator Previous( const TokenIterator& TokenPosition );
TokenIterator Next( const TokenIterator& TokenPosition );
bool AreInSameLine( Token* T1, Token*T2 );


// =============================================================================
//      CHIP-8 PARSER
// =============================================================================


class VirconASMParser
{
    protected:
        
        // source data. Not a link, but actually a copy!
        // since it needs to be modified to replace variables
        TokenList Tokens;

    public:
        
        // results
        NodeList ProgramAST;
        
    protected:
        
        // error handling
        void EmitError( SourceLocation Location, const std::string& Description, bool Abort = true );
        void EmitWarning( SourceLocation Location, const std::string& Description );
        
        // miscellaneous functions
        void ExpectSameLine( Token* Start, Token* Current );
        void ExpectEndOfLine( Token* Start, Token* Current );
        
        // contextual parsing functions
        BasicValue ParseBasicValue( TokenIterator& TokenPosition );
        InstructionOperand ParseOperand( TokenIterator& TokenPosition );
        InstructionNode* ParseInstruction( TokenIterator& TokenPosition );
        IntegerDataNode* ParseIntegerData( TokenIterator& TokenPosition );
        FloatDataNode* ParseFloatData( TokenIterator& TokenPosition );
        StringDataNode* ParseStringData( TokenIterator& TokenPosition );
        PointerDataNode* ParsePointerData( TokenIterator& TokenPosition );
        LabelNode* ParseLabel( TokenIterator& TokenPosition );
        DataFileNode* ParseDataFile( TokenIterator& TokenPosition );
        
    public:
        
        // instance handling
       ~VirconASMParser();
        
        // main parsing function
        void ParseTopLevel( TokenList& Tokens_ );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
