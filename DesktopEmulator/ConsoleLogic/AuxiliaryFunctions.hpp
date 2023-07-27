// *****************************************************************************
    // start include guard
    #ifndef AUXILIARYFUNCTIONS_HPP
    #define AUXILIARYFUNCTIONS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      FILE HANDLING FUNCTIONS
    // =============================================================================
    
    
    extern char PathSeparator;
    std::string GetPathFileName( const std::string& FilePath );
    
    
    // =============================================================================
    //      SIGNATURE HANDLING FUNCTIONS
    // =============================================================================
    
    
    void WriteSignature( std::ostream& OutputFile, const char* Value );
    bool CheckSignature( char* Signature, const char* Expected );
    
    
    // =============================================================================
    //      NUMERIC FUNCTIONS
    // =============================================================================
    
    
    template<typename T>
    void Clamp( T& Variable, int Mininum, int Maximum )
    {
        if( Variable > Maximum ) Variable = Maximum;
        if( Variable < Mininum ) Variable = Mininum;
    }
    
    // -----------------------------------------------------------------------------
    
    template<typename T>
    void Clamp( T& Variable, double Mininum, double Maximum )
    {
        if( Variable > Maximum ) Variable = Maximum;
        if( Variable < Mininum ) Variable = Mininum;
    }
    
    // -----------------------------------------------------------------------------
    
    inline bool IsBetween( double Value, double Minimum, double Maximum )
    {
        return (Value >= Minimum) && (Value <= Maximum);
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
