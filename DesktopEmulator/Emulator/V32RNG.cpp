// *****************************************************************************
    // include project headers
    #include "V32RNG.hpp"
    
    // include C/C++ headers
    #include <stdlib.h>     // [ ANSI C ] Standard library
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CLASS: V32 RNG
    // =============================================================================
    
    
    V32RNG::V32RNG()
    {
        // do not use a seed value of 0!
        CurrentValue = 1;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32RNG::ReadPort( int32_t LocalPort, VirconWord& Result )
    {
        // check range
        if( LocalPort != (int32_t)RNG_LocalPorts::CurrentValue )
          return false;
        
        // provide value
        Result.AsInteger = CurrentValue;
        
        // determine next value, with the formula
        // of a linear congruential generator
        // (values here are taken from C++11's minstd_rand)
        int64_t Aux = CurrentValue;
        Aux *= 48271;
        CurrentValue = Aux % 0x7FFFFFFF;
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    bool V32RNG::WritePort( int32_t LocalPort, VirconWord Value )
    {
        if( LocalPort != (int32_t)RNG_LocalPorts::CurrentValue )
          return false;
        
        // value 0 would produce a sequence of zeroes, so
        // just silently refuse to write that particular value
        if( Value.AsInteger == 0 )
          return true;
        
        // write value, disregarding the sign bit
        // (don't just use abs, since negative range
        // is higher and it can overflow our variable)
        VirconWord ConvertedValue;
        ConvertedValue.AsInteger = Value.AsInteger;
        ConvertedValue.AsBinary &= 0x7FFFFFFF;
        
        CurrentValue = abs( ConvertedValue.AsInteger );
        
        return true;
    }
    
    // -----------------------------------------------------------------------------
    
    void V32RNG::Reset()
    {
        // initialize seed
        // do not use a seed value of 0!
        CurrentValue = 1;
    }
}
