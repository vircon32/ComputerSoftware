// *****************************************************************************
    // start include guard
    #ifndef V32SPU_HPP
    #define V32SPU_HPP
    
    // include common vircon32 headers
    #include "../VirconDefinitions/Enumerations.hpp"
    
    // include console logic headers
    #include "V32Buses.hpp"
    #include "ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      SPU DEFINITIONS
    // =============================================================================
    
    
    // LOCAL port numbers!
    // (to be added the initial address)
    enum class SPU_LocalPorts: int32_t
    {
        Command = 0,
        
        GlobalVolume,
        SelectedSound,
        SelectedChannel,
        
        SoundLength,
        SoundPlayWithLoop,
        SoundLoopStart,
        SoundLoopEnd,
        
        ChannelState,
        ChannelAssignedSound,
        ChannelVolume,
        ChannelSpeed,
        ChannelLoopEnabled,
        ChannelPosition
    };
    
    // used as limit of local port numbers
    const int32_t SPU_LastPort = (int32_t)SPU_LocalPorts::ChannelPosition;
    
    
    // =============================================================================
    //      SPU DATA STRUCTURES
    // =============================================================================
    
    
    typedef struct
    {
        // accesible ports
        int32_t Length;
        int32_t PlayWithLoop;
        int32_t LoopStart;
        int32_t LoopEnd;
        
        // actual sound samples
        std::vector< SPUSample > Samples;
    }
    SPUSound;
    
    // -----------------------------------------------------------------------------
    
    typedef struct
    {
        IOPortValues State;
        int32_t AssignedSound;
        float Volume;
        float Speed;
        int32_t LoopEnabled;
        
        // other needed fields
        double Position;         // we need double for extra range and precision
        SPUSound* CurrentSound;  // working pointer for speed
    }
    SPUChannel;
    
    
    // =============================================================================
    //      V32 SPU CLASS
    // =============================================================================
    
    
    class V32SPU: public VirconControlInterface
    {
        public:
            
            // sounds loaded into SPU
            SPUSound BiosSound;
            SPUSound CartridgeSounds[ Constants::SPUMaximumCartridgeSounds ];
            unsigned LoadedCartridgeSounds;
            
            // SPU registers
            int32_t Command;
            float GlobalVolume;
            int32_t SelectedSound;
            int32_t SelectedChannel;
            
            // accessors to active entities
            SPUSound*   PointedSound;
            SPUChannel* PointedChannel;
            
            // sound channels
            SPUChannel Channels[ Constants::SPUSoundChannels ];
            
            // sound buffer configuration
            SPUOutputBuffer OutputBuffer;
            
        public:
            
            // instance handling
            V32SPU();
           ~V32SPU();
            
            // handling of audio resources
            void LoadSound( SPUSound& TargetSound, SPUSample* Samples, unsigned NumberOfSamples );
            void UnloadSound( SPUSound& TargetSound );
            
            // I/O bus connection
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
            
            // general operation
            void ChangeFrame();
            void Reset();
            
            // execution of GPU commands
            void PlayChannel ( SPUChannel& TargetChannel );
            void PauseChannel( SPUChannel& TargetChannel );
            void StopChannel ( SPUChannel& TargetChannel );
            void PauseAllChannels();
            void ResumeAllChannels();
            void StopAllChannels();
            
            // generate output sound
            void UpdateOutputBuffer();
    };
    
    
    // =============================================================================
    //      SPU REGISTER WRITERS
    // =============================================================================
    
    
    bool WriteSPUCommand              ( V32SPU& SPU, V32Word Value );
    bool WriteSPUGlobalVolume         ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSelectedSound        ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSelectedChannel      ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSoundLength          ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSoundPlayWithLoop    ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSoundLoopStart       ( V32SPU& SPU, V32Word Value );
    bool WriteSPUSoundLoopEnd         ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelState         ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelAssignedSound ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelVolume        ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelSpeed         ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelLoopEnabled   ( V32SPU& SPU, V32Word Value );
    bool WriteSPUChannelPosition      ( V32SPU& SPU, V32Word Value );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

