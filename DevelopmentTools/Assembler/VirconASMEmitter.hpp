// *****************************************************************************
    // start include guard
    #ifndef VIRCONASMEMITTER_HPP
    #define VIRCONASMEMITTER_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include project headers
    #include "ASTNodes.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
// *****************************************************************************


// =============================================================================
//      EMIT FROM GENERIC INSTRUCTION NODE
// =============================================================================


class VirconASMEmitter
{
    protected:
        
        // link to source data
        NodeList* ProgramAST;
        
    public:
        
        // results
        std::vector< V32::V32Word > ROM;
        std::map< std::string, int32_t > LabelAddresses;
        
    public:
        
        // error handling
        void EmitError( int LineInSource, const std::string& Description, bool Abort = true );
        void EmitWarning( int LineInSource, const std::string& Description );
        
        // helpers for emit functions
        void CheckOperands( InstructionNode& Node, int NumberOfOperands );
        int32_t GetLabelAddress( ASTNode& ReferringNode, std::string LabelName );
        void ReadDataFile( DataFileNode& Node );
        
        int32_t         GetValueAsAddress  ( InstructionNode& Node, BasicValue& Value );
        V32::V32Word GetValueAsImmediate( InstructionNode& Node, BasicValue& Value );
        
        // emit function for an instruction node
        void EmitInstructionFromNode( InstructionNode& Node );
        
    public:
        
        // instance handling
        VirconASMEmitter();
        
        // main assembly function
        void Emit( NodeList& ProgramAST_ );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
