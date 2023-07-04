// *****************************************************************************
    // start include guard
    #ifndef ENUMSTRINGCONVERSIONS_HPP
    #define ENUMSTRINGCONVERSIONS_HPP
    
    // include Vircon32 spec headers
    // include common Vircon headers
    #include "../../VirconDefinitions/Enumerations.hpp"
    
    // include C/C++ headers
    #include <string>      // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      CONVERSIONS ENUM --> STRING
// =============================================================================


std::string OpCodeToString( V32::InstructionOpCodes OpCode );
std::string RegisterToString( V32::CPURegisters Register );
std::string PortToString( V32::IOPorts Port );
std::string PortValueToString( V32::IOPortValues Port );

// context-dependent conversion for specific port values
std::string GPUCommandToString( V32::IOPortValues PortValue );
std::string SPUCommandToString( V32::IOPortValues PortValue );
std::string GPUBlendingModeToString( V32::IOPortValues PortValue );
std::string SPUChannelStateToString( V32::IOPortValues PortValue );


// =============================================================================
//      CONVERSIONS STRING --> ENUM
// =============================================================================


// detection of enum type from a string
bool IsOpCodeName( std::string Word );
bool IsRegisterName( std::string Word );
bool IsPortName( std::string Word );
bool IsPortValueName( std::string Word );

// actual conversion to enum values
// (when not valid, a string with the hex number will be returned)
V32::InstructionOpCodes StringToOpCode( std::string& Word );
V32::CPURegisters StringToRegister( std::string& Word );
V32::IOPorts StringToPort( std::string& Word );
V32::IOPortValues StringToPortValue( std::string& Word );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
