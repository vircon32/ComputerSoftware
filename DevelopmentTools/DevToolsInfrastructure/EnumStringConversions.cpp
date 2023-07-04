// *****************************************************************************
    // include project headers
    #include "EnumStringConversions.hpp"
    #include "StringFunctions.hpp"
    
    // include C/C++ headers
    #include <iomanip>      // [ C++ STL ] I/O Manipulation
    #include <iostream>     // [ C++ STL ] I/O Streams
    #include <map>          // [ C++ STL ] Maps
    #include <stdexcept>    // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      ENUM --> STRING MAPS
// =============================================================================


const map< InstructionOpCodes, string > OpCodeNames =
{
    { InstructionOpCodes::HLT,   "HLT"   },
    { InstructionOpCodes::WAIT,  "WAIT"  },
    
    { InstructionOpCodes::JMP,   "JMP"   },
    { InstructionOpCodes::CALL,  "CALL"  },
    { InstructionOpCodes::RET,   "RET"   },
    { InstructionOpCodes::JT,    "JT"    },
    { InstructionOpCodes::JF,    "JF"    },
    
    { InstructionOpCodes::IEQ,   "IEQ"   },
    { InstructionOpCodes::INE,   "INE"   },
    { InstructionOpCodes::IGT,   "IGT"   },
    { InstructionOpCodes::IGE,   "IGE"   },
    { InstructionOpCodes::ILT,   "ILT"   },
    { InstructionOpCodes::ILE,   "ILE"   },
    
    { InstructionOpCodes::FEQ,   "FEQ"   },
    { InstructionOpCodes::FNE,   "FNE"   },
    { InstructionOpCodes::FGT,   "FGT"   },
    { InstructionOpCodes::FGE,   "FGE"   },
    { InstructionOpCodes::FLT,   "FLT"   },
    { InstructionOpCodes::FLE,   "FLE"   },
    
    { InstructionOpCodes::MOV,   "MOV"   },
    { InstructionOpCodes::LEA,   "LEA"   },
    { InstructionOpCodes::PUSH,  "PUSH"  },
    { InstructionOpCodes::POP,   "POP"   },
    { InstructionOpCodes::IN,    "IN"    },
    { InstructionOpCodes::OUT,   "OUT"   },
    
    { InstructionOpCodes::MOVS,  "MOVS"  },
    { InstructionOpCodes::SETS,  "SETS"  },
    { InstructionOpCodes::CMPS,  "CMPS"  },
    
    { InstructionOpCodes::CIF,   "CIF"   },
    { InstructionOpCodes::CFI,   "CFI"   },
    { InstructionOpCodes::CIB,   "CIB"   },
    { InstructionOpCodes::CFB,   "CFB"   },
    
    { InstructionOpCodes::NOT,   "NOT"   },
    { InstructionOpCodes::AND,   "AND"   },
    { InstructionOpCodes::OR,    "OR"    },
    { InstructionOpCodes::XOR,   "XOR"   },
    { InstructionOpCodes::BNOT,  "BNOT"  },
    { InstructionOpCodes::SHL,   "SHL"   },
    
    { InstructionOpCodes::IADD,  "IADD"  },
    { InstructionOpCodes::ISUB,  "ISUB"  },
    { InstructionOpCodes::IMUL,  "IMUL"  },
    { InstructionOpCodes::IDIV,  "IDIV"  },
    { InstructionOpCodes::IMOD,  "IMOD"  },
    { InstructionOpCodes::ISGN,  "ISGN"  },
    { InstructionOpCodes::IMIN,  "IMIN"  },
    { InstructionOpCodes::IMAX,  "IMAX"  },
    { InstructionOpCodes::IABS,  "IABS"  },
    
    { InstructionOpCodes::FADD,  "FADD"  },
    { InstructionOpCodes::FSUB,  "FSUB"  },
    { InstructionOpCodes::FMUL,  "FMUL"  },
    { InstructionOpCodes::FDIV,  "FDIV"  },
    { InstructionOpCodes::FMOD,  "FMOD"  },
    { InstructionOpCodes::FSGN,  "FSGN"  },
    { InstructionOpCodes::FMIN,  "FMIN"  },
    { InstructionOpCodes::FMAX,  "FMAX"  },
    { InstructionOpCodes::FABS,  "FABS"  },
    
    { InstructionOpCodes::FLR,   "FLR"   },
    { InstructionOpCodes::CEIL,  "CEIL"  },
    { InstructionOpCodes::ROUND, "ROUND" },
    { InstructionOpCodes::SIN,   "SIN"   },
    { InstructionOpCodes::ACOS,  "ACOS"  },
    { InstructionOpCodes::ATAN2, "ATAN2" },
    { InstructionOpCodes::LOG,   "LOG"   },
    { InstructionOpCodes::POW,   "POW"   }
};

// -----------------------------------------------------------------------------

const map< CPURegisters, string > RegisterNames =
{
    { CPURegisters::Register00,  "R0"  },
    { CPURegisters::Register01,  "R1"  },
    { CPURegisters::Register02,  "R2"  },
    { CPURegisters::Register03,  "R3"  },
    { CPURegisters::Register04,  "R4"  },
    { CPURegisters::Register05,  "R5"  },
    { CPURegisters::Register06,  "R6"  },
    { CPURegisters::Register07,  "R7"  },
    { CPURegisters::Register08,  "R8"  },
    { CPURegisters::Register09,  "R9"  },
    { CPURegisters::Register10,  "R10" },
    { CPURegisters::Register11,  "CR"  },
    { CPURegisters::Register12,  "SR"  },
    { CPURegisters::Register13,  "DR"  },
    { CPURegisters::Register14,  "BP"  },
    { CPURegisters::Register15,  "SP"  }
    
    // Careful! Since this is a map, we cannot include
    // aliases for R11 to R15 (they are repeated values)
    // so we will prefer their specific names
};

// -----------------------------------------------------------------------------

const map< IOPorts, string > PortNames =
{
    { IOPorts::TIM_CurrentDate,          "TIM_CurrentDate"          },
    { IOPorts::TIM_CurrentTime,          "TIM_CurrentTime"          },
    { IOPorts::TIM_FrameCounter,         "TIM_FrameCounter"         },
    { IOPorts::TIM_CycleCounter,         "TIM_CycleCounter"         },
    
    { IOPorts::RNG_CurrentValue,         "RNG_CurrentValue"         },
    
    { IOPorts::GPU_Command,              "GPU_Command"              },
    { IOPorts::GPU_RemainingPixels,      "GPU_RemainingPixels"      },
    { IOPorts::GPU_ClearColor,           "GPU_ClearColor"           },
    { IOPorts::GPU_MultiplyColor,        "GPU_MultiplyColor"        },
    { IOPorts::GPU_ActiveBlending,       "GPU_ActiveBlending"       },
    { IOPorts::GPU_SelectedTexture,      "GPU_SelectedTexture"      },
    { IOPorts::GPU_SelectedRegion,       "GPU_SelectedRegion"       },
    { IOPorts::GPU_DrawingPointX,        "GPU_DrawingPointX"        },
    { IOPorts::GPU_DrawingPointY,        "GPU_DrawingPointY"        },
    { IOPorts::GPU_DrawingScaleX,        "GPU_DrawingScaleX"        },
    { IOPorts::GPU_DrawingScaleY,        "GPU_DrawingScaleY"        },
    { IOPorts::GPU_DrawingAngle,         "GPU_DrawingAngle"         },
    { IOPorts::GPU_RegionMinX,           "GPU_RegionMinX"           },
    { IOPorts::GPU_RegionMinY,           "GPU_RegionMinY"           },
    { IOPorts::GPU_RegionMaxX,           "GPU_RegionMaxX"           },
    { IOPorts::GPU_RegionMaxY,           "GPU_RegionMaxY"           },
    { IOPorts::GPU_RegionHotspotX,       "GPU_RegionHotspotX"       },
    { IOPorts::GPU_RegionHotspotY,       "GPU_RegionHotspotY"       },
    
    { IOPorts::SPU_Command,              "SPU_Command"              },
    { IOPorts::SPU_GlobalVolume,         "SPU_GlobalVolume"         },
    { IOPorts::SPU_SelectedSound,        "SPU_SelectedSound"        },
    { IOPorts::SPU_SelectedChannel,      "SPU_SelectedChannel"      },
    { IOPorts::SPU_SoundLength,          "SPU_SoundLength"          },
    { IOPorts::SPU_SoundPlayWithLoop,    "SPU_SoundPlayWithLoop"    },
    { IOPorts::SPU_SoundLoopStart,       "SPU_SoundLoopStart"       },
    { IOPorts::SPU_SoundLoopEnd,         "SPU_SoundLoopEnd"         },
    { IOPorts::SPU_ChannelState,         "SPU_ChannelState"         },
    { IOPorts::SPU_ChannelAssignedSound, "SPU_ChannelAssignedSound" },
    { IOPorts::SPU_ChannelVolume,        "SPU_ChannelVolume"        },
    { IOPorts::SPU_ChannelSpeed,         "SPU_ChannelSpeed"         },
    { IOPorts::SPU_ChannelLoopEnabled,   "SPU_ChannelLoopEnabled"   },
    { IOPorts::SPU_ChannelPosition,      "SPU_ChannelPosition"      },
    
    { IOPorts::INP_SelectedGamepad,      "INP_SelectedGamepad"      },
    { IOPorts::INP_GamepadConnected,     "INP_GamepadConnected"     },
    { IOPorts::INP_GamepadLeft,          "INP_GamepadLeft"          },
    { IOPorts::INP_GamepadRight,         "INP_GamepadRight"         },
    { IOPorts::INP_GamepadUp,            "INP_GamepadUp"            },
    { IOPorts::INP_GamepadDown,          "INP_GamepadDown"          },
    { IOPorts::INP_GamepadButtonStart,   "INP_GamepadButtonStart"   },
    { IOPorts::INP_GamepadButtonA,       "INP_GamepadButtonA"       },
    { IOPorts::INP_GamepadButtonB,       "INP_GamepadButtonB"       },
    { IOPorts::INP_GamepadButtonX,       "INP_GamepadButtonX"       },
    { IOPorts::INP_GamepadButtonY,       "INP_GamepadButtonY"       },
    { IOPorts::INP_GamepadButtonL,       "INP_GamepadButtonL"       },
    { IOPorts::INP_GamepadButtonR,       "INP_GamepadButtonR"       },
    
    { IOPorts::CAR_Connected,            "CAR_Connected"            },
    { IOPorts::CAR_ProgramROMSize,       "CAR_ProgramROMSize"       },
    { IOPorts::CAR_NumberOfTextures,     "CAR_NumberOfTextures"     },
    { IOPorts::CAR_NumberOfSounds,       "CAR_NumberOfSounds"       },
    
    { IOPorts::MEM_Connected,            "MEM_Connected"            }
};

// -----------------------------------------------------------------------------

const map< IOPortValues, string > PortValueNames =
{
    { IOPortValues::GPUCommand_ClearScreen,          "GPUCommand_ClearScreen"          },
    { IOPortValues::GPUCommand_DrawRegion,           "GPUCommand_DrawRegion"           },
    { IOPortValues::GPUCommand_DrawRegionZoomed,     "GPUCommand_DrawRegionZoomed"     },
    { IOPortValues::GPUCommand_DrawRegionRotated,    "GPUCommand_DrawRegionRotated"    },
    { IOPortValues::GPUCommand_DrawRegionRotozoomed, "GPUCommand_DrawRegionRotozoomed" },
    
    { IOPortValues::GPUBlendingMode_Alpha,           "GPUBlendingMode_Alpha"           },
    { IOPortValues::GPUBlendingMode_Add,             "GPUBlendingMode_Add"             },
    { IOPortValues::GPUBlendingMode_Subtract,        "GPUBlendingMode_Subtract"        },
    
    { IOPortValues::SPUCommand_PlaySelectedChannel,  "SPUCommand_PlaySelectedChannel"  },
    { IOPortValues::SPUCommand_PauseSelectedChannel, "SPUCommand_PauseSelectedChannel" },
    { IOPortValues::SPUCommand_StopSelectedChannel,  "SPUCommand_StopSelectedChannel"  },
    { IOPortValues::SPUCommand_PauseAllChannels,     "SPUCommand_PauseAllChannels"     },
    { IOPortValues::SPUCommand_ResumeAllChannels,    "SPUCommand_ResumeAllChannels"    },
    { IOPortValues::SPUCommand_StopAllChannels,      "SPUCommand_StopAllChannels"      },
    
    { IOPortValues::SPUChannelState_Stopped,         "SPUChannelState_Stopped"         },
    { IOPortValues::SPUChannelState_Paused,          "SPUChannelState_Paused "         },
    { IOPortValues::SPUChannelState_Playing,         "SPUChannelState_Playing"         }
};


// =============================================================================
//      CONVERSIONS ENUM --> STRING
// =============================================================================


string OpCodeToString( InstructionOpCodes OpCode )
{
    // just search in the map
    auto MapPair = OpCodeNames.find( OpCode );
    if( MapPair != OpCodeNames.end() )
      return MapPair->second;
    
    // when not found, provide the number in hex
    return Hex( (unsigned)OpCode, 2 );
}

// -----------------------------------------------------------------------------

string RegisterToString( CPURegisters Register )
{
    // just search in the map
    auto MapPair = RegisterNames.find( Register );
    if( MapPair != RegisterNames.end() )
      return MapPair->second;
    
    // when not found, provide the number in decimal
    return to_string( (unsigned)Register );
}

// -----------------------------------------------------------------------------

string PortToString( IOPorts Port )
{
    // just search in the map
    auto MapPair = PortNames.find( Port );
    if( MapPair != PortNames.end() )
      return MapPair->second;
    
    // when not found, provide the number in hex
    return Hex( (unsigned)Port, 4 );
}

// -----------------------------------------------------------------------------

string PortValueToString( IOPortValues PortValue )
{
    // just search in the map
    auto MapPair = PortValueNames.find( PortValue );
    if( MapPair != PortValueNames.end() )
      return MapPair->second;
    
    // when not found, provide the number in hex
    return Hex( (unsigned)PortValue, 4 );
}

// -----------------------------------------------------------------------------

string GPUCommandToString( IOPortValues PortValue )
{
    switch( PortValue )
    {
        case IOPortValues::GPUCommand_ClearScreen:
        case IOPortValues::GPUCommand_DrawRegion:
        case IOPortValues::GPUCommand_DrawRegionZoomed:
        case IOPortValues::GPUCommand_DrawRegionRotated:
        case IOPortValues::GPUCommand_DrawRegionRotozoomed:
        {
            auto MapPair = PortValueNames.find( PortValue );
            return MapPair->second;
        }
        
        default: break;
    }
    
    // when not found, provide the number in hex
    return Hex( (unsigned)PortValue, 8 );
}


// -----------------------------------------------------------------------------

string SPUCommandToString( IOPortValues PortValue )
{
    switch( PortValue )
    {
        case IOPortValues::SPUCommand_PlaySelectedChannel:
        case IOPortValues::SPUCommand_PauseSelectedChannel:
        case IOPortValues::SPUCommand_StopSelectedChannel:
        case IOPortValues::SPUCommand_PauseAllChannels:
        case IOPortValues::SPUCommand_ResumeAllChannels:
        case IOPortValues::SPUCommand_StopAllChannels:
        {
            auto MapPair = PortValueNames.find( PortValue );
            return MapPair->second;
        }
        
        default: break;
    }
    
    // when not found, provide the number in hex
    return Hex( (unsigned)PortValue, 8 );
}

// -----------------------------------------------------------------------------

string GPUBlendingModeToString( IOPortValues PortValue )
{
    switch( PortValue )
    {
        case IOPortValues::GPUBlendingMode_Alpha:
        case IOPortValues::GPUBlendingMode_Add:
        case IOPortValues::GPUBlendingMode_Subtract:
        {
            auto MapPair = PortValueNames.find( PortValue );
            return MapPair->second;
        }
        
        default: break;
    }
    
    // when not found, provide the number in hex
    return Hex( (unsigned)PortValue, 8 );
}

// -----------------------------------------------------------------------------

string SPUChannelStateToString( IOPortValues PortValue )
{
    switch( PortValue )
    {
        case IOPortValues::SPUChannelState_Stopped:
        case IOPortValues::SPUChannelState_Paused:
        case IOPortValues::SPUChannelState_Playing:
        {
            auto MapPair = PortValueNames.find( PortValue );
            return MapPair->second;
        }
        
        default: break;
    }
    
    // when not found, provide the number in hex
    return Hex( (unsigned)PortValue, 8 );
}


// =============================================================================
//      CONVERSIONS STRING --> ENUM
// =============================================================================


bool IsOpCodeName( std::string Word )
{
    // allow lowercase too
    string WordUpper = Word;
    for( char& c : WordUpper ) c = toupper( c );
    
    // search in the map
    for( auto MapPair : OpCodeNames )
      if( MapPair.second == WordUpper )
        return true;
    
    return false;
}

// -----------------------------------------------------------------------------

bool IsRegisterName( std::string Word )
{
    // allow lowercase too
    string WordUpper = Word;
    for( char& c : WordUpper ) c = toupper( c );
    
    // search in the map
    for( auto MapPair : RegisterNames )
      if( MapPair.second == WordUpper )
        return true;
    
    // account for register aliases
    if( WordUpper == "R11" ) return true;
    if( WordUpper == "R12" ) return true;
    if( WordUpper == "R13" ) return true;
    if( WordUpper == "R14" ) return true;
    if( WordUpper == "R15" ) return true;
    
    return false;
}

// -----------------------------------------------------------------------------

bool IsPortName( std::string Word )
{
    // allow lowercase too
    string WordUpper = Word;
    for( char& c : WordUpper ) c = toupper( c );
    
    // search in the map
    for( auto MapPair : PortNames )
    {
        string PortUpper = MapPair.second;
        for( char& c : PortUpper ) c = toupper( c );
        
        if( PortUpper == WordUpper )
          return true;
    }
    
    return false;
}

// -----------------------------------------------------------------------------

bool IsPortValueName( std::string Word )
{
    // allow lowercase too
    string WordUpper = Word;
    for( char& c : WordUpper ) c = toupper( c );
    
    // search in the map
    for( auto MapPair : PortValueNames )
    {
        string PortValueUpper = MapPair.second;
        for( char& c : PortValueUpper ) c = toupper( c );
        
        if( PortValueUpper == WordUpper )
          return true;
    }
    
    return false;
}

// -----------------------------------------------------------------------------

InstructionOpCodes StringToOpCode( string& Word )
{
    // allow lowercase too
    string NameUpper = Word;
    for( char& c : NameUpper ) c = toupper( c );
    
    // now search in the map
    for( auto MapPair : OpCodeNames )
      if( MapPair.second == NameUpper )
        return MapPair.first;
    
    // not found
    throw runtime_error( "String cannot be converted to an instruction opcode" );
}

// -----------------------------------------------------------------------------

CPURegisters StringToRegister( string& Word )
{
    // allow lowercase too
    string NameUpper = Word;
    for( char& c : NameUpper ) c = toupper( c );
    
    // now search in the map
    for( auto MapPair : RegisterNames )
      if( MapPair.second == NameUpper )
        return MapPair.first;
    
    // account for register aliases
    if( NameUpper == "R11" )  return CPURegisters::CountRegister;
    if( NameUpper == "R12" )  return CPURegisters::SourceRegister;
    if( NameUpper == "R13" )  return CPURegisters::DestinationRegister;
    if( NameUpper == "R14" )  return CPURegisters::BasePointer;
    if( NameUpper == "R15" )  return CPURegisters::StackPointer;
    
    // not found
    throw runtime_error( "String cannot be converted to a CPU register" );
}

// -----------------------------------------------------------------------------

IOPorts StringToPort( string& Word )
{
    // allow lowercase too
    string NameUpper = Word;
    for( char& c : NameUpper ) c = toupper( c );
    
    // now search in the map
    for( auto MapPair : PortNames )
    {
        string PortUpper = MapPair.second;
        for( char& c : PortUpper ) c = toupper( c );
        
        if( PortUpper == NameUpper )
          return MapPair.first;
    }
    
    // not found
    throw runtime_error( "String cannot be converted to an I/O port" );
}

// -----------------------------------------------------------------------------

IOPortValues StringToPortValue( string& Word )
{
    // allow lowercase too
    string NameUpper = Word;
    for( char& c : NameUpper ) c = toupper( c );
    
    // now search in the map
    for( auto MapPair : PortValueNames )
    {
        string PortValueUpper = MapPair.second;
        for( char& c : PortValueUpper ) c = toupper( c );
        
        if( PortValueUpper == NameUpper )
          return MapPair.first;
    }
    
    // not found
    throw runtime_error( "String cannot be converted to an I/O port value" );
}
