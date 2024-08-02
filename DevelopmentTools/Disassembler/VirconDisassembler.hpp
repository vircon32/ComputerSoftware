// *****************************************************************************
    // start include guard
    #ifndef VIRCONDISASSEMBLER_HPP
    #define VIRCONDISASSEMBLER_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/Definitions.hpp"
    
    // include project headers
    #include "Globals.hpp"
    
    // include external headers
    #include <string>       // [ C++ STL ] Strings
    #include <vector>       // [ C++ STL ] Vectors
    #include <map>          // [ C++ STL ] Maps
// *****************************************************************************


// =============================================================================
//      VIRCON DISASSEMBLER
// =============================================================================


class VirconDisassembler
{
    public:
        
        // internal intermediate results
        std::map< uint32_t, V32::CPUInstruction > VisitedInstructions;
        std::map< uint32_t, std::string > JumpDestinationNames;
        
    public:
        
        // results
        std::vector< V32::V32Word > ROM;
        
    protected:
        
        // partial disassembly function
        void DisassembleBranch( uint32_t ROMIndex );
        
    public:
        
        // main disassembly functions
        void LoadROM( const std::string& InputPath );
        void Disassemble( std::ostream& Output, bool IncludeDescriptions = false );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
