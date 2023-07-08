// *****************************************************************************
    // start include guard
    #ifndef V32TIMER_HPP
    #define V32TIMER_HPP
    
    // include project headers
    #include "V32Buses.hpp"
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      TIMER DEFINITIONS
    // =============================================================================
    
    
    // local port numbers
    enum class CLK_LocalPorts: int32_t
    {
        CurrentDate = 0,
        CurrentTime,
        FrameCounter,
        CycleCounter
    };
    
    // used as limit of local port numbers
    const int32_t CLK_LastPort = (int32_t)CLK_LocalPorts::CycleCounter;
    
    
    // =============================================================================
    //      V32 TIME MANAGER
    // =============================================================================
    
    
    class V32Timer: public VirconControlInterface
    {
        public:
            
            int32_t CurrentDate;
            int32_t CurrentTime;
            int32_t FrameCounter;
            int32_t CycleCounter;
            
        public:
            
            // instance handling
            V32Timer();
            
            // connection to control bus
            virtual bool ReadPort( int32_t LocalPort, V32Word& Result );
            virtual bool WritePort( int32_t LocalPort, V32Word Value );
            
            // general operation
            void RunNextCycle();
            void ChangeFrame();
            void Reset();
    };
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
