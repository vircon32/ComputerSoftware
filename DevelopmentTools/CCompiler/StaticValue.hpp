// *****************************************************************************
    // start include guard
    #ifndef STATICVALUE_HPP
    #define STATICVALUE_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include project headers
    #include "DataTypes.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      HOLDER FOR RESULTS OF STATIC CALCULATIONS
// =============================================================================


class StaticValue
{
    public:
        
        V32::VirconWord Word;
        PrimitiveTypes Type;    // static values can only be primitives
        
    public:
        
        StaticValue( bool Value );
        StaticValue( int32_t Value );
        StaticValue( uint32_t Value );
        StaticValue( float Value );
        
        std::string ToString();
        void ConvertToType( PrimitiveTypes NewType );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
