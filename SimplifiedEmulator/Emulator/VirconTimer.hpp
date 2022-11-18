// *****************************************************************************
    // start include guard
    #ifndef VIRCONTIMER_HPP
    #define VIRCONTIMER_HPP
    
    // include project headers
    #include "VirconBuses.hpp"
// *****************************************************************************


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
//      VIRCON TIME MANAGER
// =============================================================================


class VirconTimer: public VirconControlInterface
{
    public:
        
        int32_t CurrentDate;
        int32_t CurrentTime;
        int32_t FrameCounter;
        int32_t CycleCounter;
        
    public:
        
        // instance handling
        VirconTimer();
        
        // connection to control bus
        virtual bool ReadPort( int32_t LocalPort, VirconWord& Result );
        virtual bool WritePort( int32_t LocalPort, VirconWord Value );
        
        // general operation
        void RunNextCycle();
        void ChangeFrame();
        void Reset();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
