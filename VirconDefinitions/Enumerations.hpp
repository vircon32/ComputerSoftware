// *****************************************************************************
    // start include guard
    #ifndef V32ENUMERATIONS_HPP
    #define V32ENUMERATIONS_HPP
    
    // include project headers
    #include "Constants.hpp"
    
    // include C/C++ headers
    #include <cstdint>       // [ ANSI C ] Standard integer types
    
    // this fix may be needed to avoid name collisions
    // with definitions from some Windows headers
    #undef IN
    #undef OUT
// -----------------------------------------------------------------------------
//  This file defines all enumeration-based values for the
//  different Vircon32 components.
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      ENUMERATIONS FOR THE WHOLE VIRCON32 SYSTEM
    // =============================================================================
    
    
    // values encoded here are external addresses of the ports
    // (i.e. dependent on the initial addresses of each device
    // connected to the I/O bus)
    enum class IOPorts: int
    {
        // global timer
        TIM_CurrentDate = Constants::TIM_FirstPort,
        TIM_CurrentTime,
        TIM_FrameCounter,
        TIM_CycleCounter,
        
        // random number generator
        RNG_CurrentValue = Constants::RNG_FirstPort,
        
        // GPU interface registers
        GPU_Command = Constants::GPU_FirstPort,
        GPU_RemainingPixels,
        GPU_ClearColor,
        GPU_MultiplyColor,
        GPU_ActiveBlending,
        GPU_SelectedTexture,
        GPU_SelectedRegion,
        GPU_DrawingPointX,
        GPU_DrawingPointY,
        GPU_DrawingScaleX,
        GPU_DrawingScaleY,
        GPU_DrawingAngle,
        GPU_RegionMinX,
        GPU_RegionMinY,
        GPU_RegionMaxX,
        GPU_RegionMaxY,
        GPU_RegionHotspotX,
        GPU_RegionHotspotY,
        
        // SPU interface registers
        SPU_Command = Constants::SPU_FirstPort,
        SPU_GlobalVolume,
        SPU_SelectedSound,
        SPU_SelectedChannel,
        SPU_SoundLength,
        SPU_SoundPlayWithLoop,
        SPU_SoundLoopStart,
        SPU_SoundLoopEnd,
        SPU_ChannelState,
        SPU_ChannelAssignedSound,
        SPU_ChannelVolume,
        SPU_ChannelSpeed,
        SPU_ChannelLoopEnabled,
        SPU_ChannelPosition,
        
        // input controller
        INP_SelectedGamepad = Constants::INP_FirstPort,
        INP_GamepadConnected,
        INP_GamepadLeft,
        INP_GamepadRight,
        INP_GamepadUp,
        INP_GamepadDown,
        INP_GamepadButtonStart,
        INP_GamepadButtonA,
        INP_GamepadButtonB,
        INP_GamepadButtonX,
        INP_GamepadButtonY,
        INP_GamepadButtonL,
        INP_GamepadButtonR,
        
        // cartridge controller
        CAR_Connected = Constants::CAR_FirstPort,
        CAR_ProgramROMSize,
        CAR_NumberOfTextures,
        CAR_NumberOfSounds,
        
        // memory card controller
        MEM_Connected = Constants::MEM_FirstPort
    };
    
    // -----------------------------------------------------------------------------
    
    // these are all the values accepted by I/O ports
    // that have specific, non-numeric values
    enum class IOPortValues: int32_t
    {
        // commands accepted by GPU Command port
        GPUCommand_ClearScreen = 0x10,         // clears the screen using current clear color
        GPUCommand_DrawRegion,                 // draws the selected region: Rotation off, Zoom off
        GPUCommand_DrawRegionZoomed,           // draws the selected region: Rotation off, Zoom on 
        GPUCommand_DrawRegionRotated,          // draws the selected region: Rotation on , Zoom off
        GPUCommand_DrawRegionRotozoomed,       // draws the selected region: Rotation on , Zoom on
        
        // modes accepted by GPU Active Blending port
        GPUBlendingMode_Alpha = 0x20,          // default rendering, uses alpha channel as transparency
        GPUBlendingMode_Add,                   // colors are added (light effect), also called linear dodge
        GPUBlendingMode_Subtract,              // colors are subtracted (shadow effect), also called difference
        
        // commands accepted by SPU Command port
        SPUCommand_PlaySelectedChannel = 0x30, // if paused, it is resumed; if already playing, it is retriggered
        SPUCommand_PauseSelectedChannel,       // no effect if the channel was not playing
        SPUCommand_StopSelectedChannel,        // position is rewinded to sound start
        SPUCommand_PauseAllChannels,           // same as applying PauseChannel to all channels
        SPUCommand_ResumeAllChannels,          // same as applying PlayChannel to all paused channels
        SPUCommand_StopAllChannels,            // same as applying StopChannel to all channels
        
        // states accepted by SPU Channel State port
        SPUChannelState_Stopped = 0x40,        // channel is not playing, and will begin new reproduction on play
        SPUChannelState_Paused,                // channel is paused, and will resume reproduction on play
        SPUChannelState_Playing                // channel is currently playing, until its assigned sound ends
    };
    
    
    // =============================================================================
    //      ENUMERATIONS SPECIFIC FOR THE CPU
    // =============================================================================
    
    
    // there are 64 CPU opcodes, so instructions encode
    // them in 6 bits. No invalid opcodes can exist.
    // HLT is opcode 0 for safety: if an empty or invalid
    // instruction is found, the CPU will stop execution.
    enum class InstructionOpCodes: int
    {
        // CPU control
        HLT = 0,    // Halt CPU operation
        WAIT,       // Wait for next frame
        
        // jump instructions
        JMP,        // Jump inconditionally
        CALL,       // Call subroutine
        RET,        // Return from subroutine
        JT,         // Jump if true
        JF,         // Jump if false
        
        // integer comparisons
        IEQ,        // Integer Equal
        INE,        // Integer Not Equal
        IGT,        // Integer Greater Than
        IGE,        // Integer Greater or Equal
        ILT,        // Integer Less Than
        ILE,        // Integer Less or Equal
        
        // float comparisons
        FEQ,        // Float Equal
        FNE,        // Float Not Equal
        FGT,        // Float Greater Than
        FGE,        // Float Greater or Equal
        FLT,        // Float Less Than
        FLE,        // Float Less or Equal
        
        // data movement
        MOV,        // Move data
        LEA,        // Load effective address of a memory position
        PUSH,       // Save on top of the stack
        POP,        // Load from top of the stack
        IN,         // Read from an I/O port
        OUT,        // Write to an I/O port
        
        // string operations
        MOVS,       // Copy string (HW memcpy)
        SETS,       // Set string (HW memset)
        CMPS,       // Compare string (HW memcmp)
        
        // data conversion
        CIF,        // Convert Integer to Float
        CFI,        // Convert Float to Integer
        CIB,        // Convert Integer to Boolean
        CFB,        // Convert Float to Boolean
        
        // binary operations
        NOT,        // Bitwise NOT
        AND,        // Bitwise AND
        OR,         // Bitwise OR
        XOR,        // Bitwise XOR
        BNOT,       // Boolean NOT
        SHL,        // Bit shift left
        
        // integer arithmetic
        IADD,       // Integer Addition
        ISUB,       // Integer Subtraction
        IMUL,       // Integer Multiplication
        IDIV,       // Integer Division
        IMOD,       // Integer Modulus
        ISGN,       // Integer Sign change
        IMIN,       // Integer Minimum
        IMAX,       // Integer Maximum
        IABS,       // Integer Absolute value
        
        // float arithmetic
        FADD,       // Float Addition
        FSUB,       // Float Subtraction
        FMUL,       // Float Multiplication
        FDIV,       // Float Division
        FMOD,       // Float Modulus
        FSGN,       // Float Sign change
        FMIN,       // Float Minimum
        FMAX,       // Float Maximum
        FABS,       // Float Absolute value
        
        // extended float operations
        FLR,        // Round down
        CEIL,       // Round up
        ROUND,      // Round to nearest integer
        SIN,        // Sine
        ACOS,       // Arc cosine
        ATAN2,      // Arc Tangent from x and y
        LOG,        // Natural logarithm
        POW         // Raise to a Power
    };
    
    // -----------------------------------------------------------------------------
    
    enum class CPURegisters: int
    {
        // all 16 general-purpose registers
        Register00 = 0,
        Register01,
        Register02,
        Register03,
        Register04,
        Register05,
        Register06,
        Register07,
        Register08,
        Register09,
        Register10,
        Register11,
        Register12,
        Register13,
        Register14,
        Register15,
        
        // alternate names for specific registers
        CountRegister       = 11,
        SourceRegister      = 12,
        DestinationRegister = 13,
        BasePointer         = 14,
        StackPointer        = 15
    };
    
    // -----------------------------------------------------------------------------
    
    enum class AddressingModes : unsigned int
    {
        RegisterFromImmediate = 0,      // syntax: MOV R1, 25
        RegisterFromRegister,           // syntax: MOV R1, R2
        RegisterFromImmediateAddress,   // syntax: MOV R1, [25]
        RegisterFromRegisterAddress,    // syntax: MOV R1, [R2]
        RegisterFromAddressOffset,      // syntax: MOV R1, [R2+25]
        ImmediateAddressFromRegister,   // syntax: MOV [25], R2
        RegisterAddressFromRegister,    // syntax: MOV [R1], R2
        AddressOffsetFromRegister       // syntax: MOV [R1+25], R2
    };
    
    // -----------------------------------------------------------------------------
    
    enum class CPUErrorCodes: uint32_t
    {
        InvalidMemoryRead = 0,
        InvalidMemoryWrite,
        InvalidPortRead,
        InvalidPortWrite,
        StackOverflow,
        StackUnderflow,
        DivisionError,
        ArcCosineError,
        ArcTangent2Error,
        LogarithmError,
        PowerError
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
