// *****************************************************************************
    // include project files
    #include "CheckUnaryOperations.hpp"
    #include "CompilerInfrastructure.hpp"
// *****************************************************************************


// =============================================================================
//      SEPARATE CHECKS FOR EACH UNARY OPERATOR
// =============================================================================


void CheckPlusSign( UnaryOperationNode* Operation )
{
    // operator needs to be a number
    DataType* OperandType = Operation->Operand->ReturnedType;
    
    if( !TypeIsNumeric( OperandType ) )
      RaiseError( Operation->Location, "plus sign can only apply to number types" );
}

// -----------------------------------------------------------------------------

void CheckMinusSign( UnaryOperationNode* Operation )
{
    // operator needs to be a number
    DataType* OperandType = Operation->Operand->ReturnedType;
    
    if( !TypeIsNumeric( OperandType ) )
      RaiseError( Operation->Location, "minus sign can only apply to number types" );
}

// -----------------------------------------------------------------------------

void CheckIncrement( UnaryOperationNode* Operation )
{
    // operand needs to be a pointer or a number
    DataType* OperandType = Operation->Operand->ReturnedType;
    bool OperandIsNumber = TypeIsNumeric( OperandType );
    bool OperandIsPointer = (OperandType->Type() == DataTypes::Pointer);
    
    if( !OperandIsNumber && !OperandIsPointer )
      RaiseError( Operation->Location, "increment can only operate with number types or pointers" );
    
    // also, the operand must be stored in memory
    if( !Operation->Operand->HasMemoryPlacement() )
      RaiseError( Operation->Location, "increment can only operate on a memory address" );
}

// -----------------------------------------------------------------------------

void CheckDecrement( UnaryOperationNode* Operation )
{
    // operand needs to be a pointer or a number
    DataType* OperandType = Operation->Operand->ReturnedType;
    bool OperandIsNumber = TypeIsNumeric( OperandType );
    bool OperandIsPointer = (OperandType->Type() == DataTypes::Pointer);
    
    if( !OperandIsNumber && !OperandIsPointer )
      RaiseError( Operation->Location, "decrement can only operate with number types or pointers" );
    
    // also, the operand must be stored in memory
    if( !Operation->Operand->HasMemoryPlacement() )
      RaiseError( Operation->Location, "decrement can only operate on a memory address" );
}

// -----------------------------------------------------------------------------

void CheckLogicalNot( UnaryOperationNode* Operation )
{
    // operand needs to be a pointer or a number
    DataType* OperandType = Operation->Operand->ReturnedType;
    bool OperandIsNumber = TypeIsNumeric( OperandType );
    bool OperandIsPointer = (OperandType->Type() == DataTypes::Pointer);
    
    if( !OperandIsNumber && !OperandIsPointer )
      RaiseError( Operation->Location, "logical NOT can only operate with number types or pointers" );
}

// -----------------------------------------------------------------------------

void CheckBitwiseNot( UnaryOperationNode* Operation )
{
    // bitwise not needs an integral type
    DataType* OperandType = Operation->Operand->ReturnedType;
    
    if( !TypeIsIntegral( OperandType ) )
      RaiseError( Operation->Location, "bitwise NOT can only operate with integral types" );
}

// -----------------------------------------------------------------------------

void CheckReference( UnaryOperationNode* Operation )
{
    // reference can only operate with memory
    if( !Operation->Operand->HasMemoryPlacement() )
      RaiseError( Operation->Location, "reference can only be applied to a memory address" );
}

// -----------------------------------------------------------------------------

void CheckDereference( UnaryOperationNode* Operation )
{
    // dereference needs a pointer
    DataType* OperandType = Operation->Operand->ReturnedType;
    
    if( OperandType->Type() != DataTypes::Pointer )
      RaiseError( Operation->Location, "dereference can only be applied to a pointer" );
      
    // void* pointers cannot be dereferenced
    // (since that would produce a void value)
    else
    {
        PointerType* OperandPointerType = (PointerType*)OperandType;
        
        if( OperandPointerType->BaseType->Type() == DataTypes::Void )
          RaiseError( Operation->Location, "pointers to void cannot be dereferenced" );
    }        
}
