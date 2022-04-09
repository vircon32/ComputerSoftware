// *****************************************************************************
    // start include guard
    #ifndef REGISTERALLOCATION_HPP
    #define REGISTERALLOCATION_HPP
    
    // include project headers
    #include "CNodes.hpp"
// *****************************************************************************


// =============================================================================
//      CLASS TO REPRESENT REGISTER ALLOCATION IN EXPRESSION EVALUATION
// =============================================================================


class RegisterAllocation
{
    public:
        
        // tracks expression in case
        // no free registers exist
        SourceLocation Location;
        
        // only registers R1 to R13 can safely be
        // allocated; R0 can be used freely by any
        // expression, while BP and SP must be
        // preserved since the CPU stack uses them
        bool RegisterUsed[ 14 ];
        
        // when using space from the allocated
        // temporaries, use a LIFO stack pattern
        int TemporariesStackSize;
        
        // this tracks the highest used register
        // (the scope is the whole root expression,
        // since the same data is passed to children)
        int HighestUsedRegister;
        
    public:
        
        RegisterAllocation( SourceLocation Location_ );
        int FirstFreeRegister();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
