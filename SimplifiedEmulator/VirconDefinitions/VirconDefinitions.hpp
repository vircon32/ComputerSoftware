// *****************************************************************************
    // start include guard
    #ifndef VIRCONDEFINITIONS_HPP
    #define VIRCONDEFINITIONS_HPP
// -----------------------------------------------------------------------------
//  This file is the most basic Vircon header.
//  It centralizes the declaration of constants.
// *****************************************************************************


// =============================================================================
//      GLOBAL VIRCON CONSTANTS
// =============================================================================


namespace Constants
{
    // version of these definitions
    const int VirconVersion  = 1;
    const int VirconRevision = 0;
    
    // timing
    const int FramesPerSecond = 60;
    const int CyclesPerSecond = 15000000;
    const int CyclesPerFrame  = CyclesPerSecond / FramesPerSecond;
    
    // display
    const int ScreenWidth  = 640;
    const int ScreenHeight = 360;
    const int ScreenPixels = ScreenWidth * ScreenHeight;
    
    // GPU specs
    const int   GPUTextureSize              = 1024;
    const int   GPUMaximumCartridgeTextures = 256;
    const int   GPURegionsPerTexture        = 4096;
    const int   GPUPixelCapacityPerFrame    = 9 * ScreenPixels;
    const float GPUClearScreenPenalty       = -0.5;
    const float GPUScalingPenalty           = +0.15;
    const float GPURotationPenalty          = +0.25;
    
    // SPU specs
    const int SPUMaximumCartridgeSounds  = 1024;
    const int SPUMaximumCartridgeSamples = 1024 * 1024 * 256;
    const int SPUMaximumBiosSamples      = 1024 * 1024 * 1;
    const int SPUSoundChannels           = 16;
    const int SPUSamplingRate            = 44100;
    
    // memory specs (all sizes in words)
    const int MaximumCartridgeProgramROM = 1024 * 1024 * 128;
    const int MaximumBiosProgramROM      = 1024 * 1024 * 1;
    const int RAMSize                    = 1024 * 1024 * 4;
    const int MemoryCardSize             = 1024 * 256;
    
    // bus specs
    const int MemoryBusSlaves  = 4;
    const int ControlBusSlaves = 8;
    
    // other specs
    const int MaximumGamepads = 4;
    
    // base memory addresses
    const int RAMFirstAddress                 = 0x00000000;
    const int BiosProgramROMFirstAddress      = 0x10000000;
    const int CartridgeProgramROMFirstAddress = 0x20000000;
    const int MemoryCardRAMFirstAddress       = 0x30000000;
    
    // base control port numbers
    const int TIM_FirstPort = 0x000;
    const int RNG_FirstPort = 0x100;
    const int GPU_FirstPort = 0x200;
    const int SPU_FirstPort = 0x300;
    const int INP_FirstPort = 0x400;
    const int CAR_FirstPort = 0x500;
    const int MEM_FirstPort = 0x600;
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
