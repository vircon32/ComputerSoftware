// *****************************************************************************
    // include project headers
    #include "StaticValue.hpp"
    
    // include C/C++ headers
    #include <stdexcept>    // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      CLASS: STATIC VALUE
// =============================================================================


StaticValue::StaticValue( int32_t Value )
{
    Type = PrimitiveTypes::Int;
    Word.AsInteger = Value;
}

// -----------------------------------------------------------------------------

StaticValue::StaticValue( uint32_t Value )
{
    Type = PrimitiveTypes::Int;
    Word.AsBinary = Value;
}

// -----------------------------------------------------------------------------

StaticValue::StaticValue( float Value )
{
    Type = PrimitiveTypes::Float;
    Word.AsFloat = Value;
}

// -----------------------------------------------------------------------------

StaticValue::StaticValue( bool Value )
{
    Type = PrimitiveTypes::Float;
    Word.AsInteger = Value;
}

// -----------------------------------------------------------------------------

void StaticValue::ConvertToType( PrimitiveTypes NewType )
{
    if( NewType == Type )
      return;
    
    if( NewType == PrimitiveTypes::Float )
    {
        if( Type == PrimitiveTypes::Int )
          Word.AsFloat = Word.AsInteger;
            
        else if( Type == PrimitiveTypes::Bool )
          Word.AsFloat = Word.AsInteger;
    }
    
    else if( NewType == PrimitiveTypes::Int )
    {
        if( Type == PrimitiveTypes::Float )
          Word.AsInteger = Word.AsFloat;
            
        // (no conversion needed for bool -> int)
    }
    
    else if( NewType == PrimitiveTypes::Bool )
    {
        if( Type == PrimitiveTypes::Float )
          Word.AsInteger = (Word.AsFloat != 0);
            
        else if( Type == PrimitiveTypes::Int )
          Word.AsInteger = (Word.AsInteger != 0);
    }
    
    // reflect the new type
    Type = NewType;
}

// -----------------------------------------------------------------------------

string StaticValue::ToString()
{
    if( Type == PrimitiveTypes::Float )
      return to_string( Word.AsFloat );
    
    // boolean numbers are just implemented as ints
    if( Type == PrimitiveTypes::Bool || Type == PrimitiveTypes::Int )
    {
        // special case: we need to write INT_MIN in
        // hex notation for the assembler to accept it
        if( Word.AsInteger == INT32_MIN )
          return "0x80000000";
        
        // other numbers can be written normally
        return to_string( Word.AsInteger );
    }
    
    // not found
    throw runtime_error( "static value cannot be converted to a string" );
}
