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
    // special case: taking the address of a named function is always valid
    if( Operation->Operand->Type() == CNodeTypes::ExpressionAtom )
      if( ((ExpressionAtomNode*)Operation->Operand)->AtomType == AtomTypes::Function )
        return;
    
    // for everything else, reference can only operate with memory
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
      
    else
    {
        PointerType* OperandPointerType = (PointerType*)OperandType;
        
        // void* pointers cannot be dereferenced
        // (since that would produce a void value)
        if( OperandPointerType->BaseType->Type() == DataTypes::Void )
          RaiseError( Operation->Location, "pointers to void cannot be dereferenced" );
        
        // function pointers can only be dereferenced when used directly
        // as the callee of an indirect call, with call syntax (*fp)()
        else if( OperandPointerType->BaseType->Type() == DataTypes::Function )
        {
            CNode* CurrentNode = Operation;
            
            while( CurrentNode->Parent && CurrentNode->Parent->IsExpression() )
            {
                if( CurrentNode->Parent->Type() == CNodeTypes::IndirectCall )
                  return;
                
                // there could be any number of parenthesis, but nothing else
                if( CurrentNode->Parent->Type() == CNodeTypes::EnclosedExpression )
                  CurrentNode = CurrentNode->Parent;
                
                else break;
            }
            
            RaiseError( Operation->Location, "function pointers can only be dereferenced when calling them" );
        }
    }        
}
