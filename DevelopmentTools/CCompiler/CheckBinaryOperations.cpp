// *****************************************************************************
    // include project files
    #include "CheckBinaryOperations.hpp"
    #include "CompilerInfrastructure.hpp"
    #include "CheckNodes.hpp"
// *****************************************************************************


// =============================================================================
//      SEPARATE CHECKS FOR EACH BINARY OPERATOR
// =============================================================================


void CheckAddition( BinaryOperationNode* Operation )
{
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    // case 1: both are number types
    bool Case1Met = TypeIsNumeric( LeftType )
                 && TypeIsNumeric( RightType );
    
    // case 2: pointer-integer arithmetic
    bool Case2AMet = (LeftType->Type() == DataTypes::Pointer)
                  && TypeIsIntegral( RightType );
    
    bool Case2BMet = (RightType->Type() == DataTypes::Pointer)
                  && TypeIsIntegral( LeftType );
    
    if( !Case1Met && !Case2AMet && !Case2BMet )
      RaiseError( Operation->Location, "invalid operands for addition" );
    
    // pointer arithmetic is only allowed when the
    // pointed elements have well defined size
    if( (Case2AMet && ((PointerType*)LeftType )->BaseType->SizeInWords() == 0)
    ||  (Case2BMet && ((PointerType*)RightType)->BaseType->SizeInWords() == 0) )
      RaiseError( Operation->Location, "pointer type is invalid for addition" );
}

// -----------------------------------------------------------------------------

void CheckSubtraction( BinaryOperationNode* Operation )
{
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    // case 1: both are number types
    bool Case1Met = TypeIsNumeric( LeftType )
                 && TypeIsNumeric( RightType );
    
    // case 2: pointer-integer arithmetic
    bool Case2Met = (LeftType->Type() == DataTypes::Pointer)
                  && TypeIsIntegral( RightType );
    
    // case 3: distance between pointers of same type
    bool Case3Met = (LeftType->Type() == DataTypes::Pointer)
                 && AreEqual( LeftType, RightType );
    
    if( !Case1Met && !Case2Met && !Case3Met )
      RaiseError( Operation->Location, "invalid operands for subtraction" );
    
    // pointer arithmetic is only allowed when the
    // pointed elements have well defined size
    if( LeftType->Type() == DataTypes::Pointer )
      if( ((PointerType*)LeftType)->BaseType->SizeInWords() == 0 )
        RaiseError( Operation->Location, "pointer type is invalid for substraction" );
}

// -----------------------------------------------------------------------------

void CheckProduct( BinaryOperationNode* Operation )
{
    // only operates on number types
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( !TypeIsNumeric( LeftType ) || !TypeIsNumeric( RightType ) )
      RaiseError( Operation->Location, "multiplication can only operate on number types" );
}

// -----------------------------------------------------------------------------

void CheckDivision( BinaryOperationNode* Operation )
{
    // only operates on number types
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( !TypeIsNumeric( LeftType ) || !TypeIsNumeric( RightType ) )
      RaiseError( Operation->Location, "division can only operate on number types" );
}

// -----------------------------------------------------------------------------

void CheckModulus( BinaryOperationNode* Operation )
{
    // all operands must be integral types
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( !TypeIsIntegral( LeftType ) || !TypeIsIntegral( RightType ) )
      RaiseError( Operation->Location, "modulus can only operate on integral types" );
}

// -----------------------------------------------------------------------------

void CheckEqual( BinaryOperationNode* Operation )
{
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    // arrays and functions cannot directly be compared
    if( LeftType->Type() == DataTypes::Array )
    {
        RaiseError( Operation->Location, "equality comparisons cannot operate on arrays" );
        return;
    }
    
    // case 1: both are number types
    bool Case1Met = TypeIsNumeric( LeftType )
                 && TypeIsNumeric( RightType );
    
    // case 2: both are equal
    bool Case2Met = AreEqual( LeftType, RightType );
    
    // case 3: all pointers can be compared with NULL i.e. integer -1
    bool Case3AMet = (LeftType->Type() == DataTypes::Pointer)
                 &&  TypeIsThisPrimitive( RightType, PrimitiveTypes::Int )
                 &&  Operation->RightOperand->IsStatic()
                 &&  (Operation->RightOperand->GetStaticValue().Word.AsInteger == -1);
    
    bool Case3BMet = (RightType->Type() == DataTypes::Pointer)
                 &&  TypeIsThisPrimitive( LeftType, PrimitiveTypes::Int )
                 &&  Operation->LeftOperand->IsStatic()
                 &&  (Operation->LeftOperand->GetStaticValue().Word.AsInteger == -1);
    
    if( !Case1Met && !Case2Met && !Case3AMet && !Case3BMet )
      RaiseError( Operation->Location, "invalid operands for equality comparison" );
}

// -----------------------------------------------------------------------------

void CheckNotEqual( BinaryOperationNode* Operation )
{
    // same checks as Equal
    CheckEqual( Operation );
}

// -----------------------------------------------------------------------------

void CheckLessThan( BinaryOperationNode* Operation )
{
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    // case 1: both are number types
    bool Case1Met = TypeIsNumeric( LeftType )
                 && TypeIsNumeric( RightType );
    
    // case 2: both are pointers of equal type
    bool Case2Met = (LeftType->Type() == DataTypes::Pointer)
                 && AreEqual( LeftType, RightType );
    
    if( !Case1Met && !Case2Met )
      RaiseError( Operation->Location, "invalid operands for arithmetic comparison" );
}

// -----------------------------------------------------------------------------

void CheckLessOrEqual( BinaryOperationNode* Operation )
{
    // same checks as LessThan
    CheckLessThan( Operation );
}

// -----------------------------------------------------------------------------

void CheckGreaterThan( BinaryOperationNode* Operation )
{
    // same checks as LessThan
    CheckLessThan( Operation );
}

// -----------------------------------------------------------------------------

void CheckGreaterOrEqual( BinaryOperationNode* Operation )
{
    // same checks as LessThan
    CheckLessThan( Operation );
}

// -----------------------------------------------------------------------------

void CheckLogicalOr( BinaryOperationNode* Operation )
{
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    // both operands need to be either numbers or pointers
    bool LeftIsValid  = TypeIsNumeric( LeftType  ) || (LeftType ->Type() == DataTypes::Pointer); 
    bool RightIsValid = TypeIsNumeric( RightType ) || (RightType->Type() == DataTypes::Pointer); 
    
    if( !LeftIsValid || !RightIsValid )
      RaiseError( Operation->Location, "invalid operands for boolean logic operator" );
}

// -----------------------------------------------------------------------------

void CheckLogicalAnd( BinaryOperationNode* Operation )
{
    // same checks as LogicalOr
    CheckLogicalOr( Operation );
}

// -----------------------------------------------------------------------------

void CheckBitwiseOr( BinaryOperationNode* Operation )
{
    // all operands must be integral types
    DataType* LeftType  = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( !TypeIsIntegral( LeftType ) || !TypeIsIntegral( RightType ) )
      RaiseError( Operation->Location, "bitwise logic operators can only operate with integral types" );
}

// -----------------------------------------------------------------------------

void CheckBitwiseAnd( BinaryOperationNode* Operation )
{
    // same checks as BitwiseOr
    CheckBitwiseOr( Operation );
}

// -----------------------------------------------------------------------------

void CheckBitwiseXor( BinaryOperationNode* Operation )
{
    // same checks as BitwiseOr
    CheckBitwiseOr( Operation );
}

// -----------------------------------------------------------------------------

void CheckShiftLeft( BinaryOperationNode* Operation )
{
    // all operands must be integral types
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( !TypeIsIntegral( LeftType ) || !TypeIsIntegral( RightType ) )
      RaiseError( Operation->Location, "bit shifts can only operate with integral types" );
}

// -----------------------------------------------------------------------------

void CheckShiftRight( BinaryOperationNode* Operation )
{
    // same checks as ShiftLeft
    CheckShiftLeft( Operation );
}

// -----------------------------------------------------------------------------

void CheckAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // types of both sides must be compatible
    CheckAssignmentTypes( Operation->Location, Operation->LeftOperand->ReturnedType, Operation->RightOperand );
}

// -----------------------------------------------------------------------------

void CheckAdditionAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // there is asymmetry! we can do pointer += integer,
    // but not integer += pointer
    if( Operation->RightOperand->ReturnedType->Type() == DataTypes::Pointer )
    {
        RaiseError( Operation->Location, "incompatible types for operator +=" );
        return;
    }
    
    // now just check the non-assignment version
    CheckAddition( Operation );
}

// -----------------------------------------------------------------------------

void CheckSubtractionAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // there is asymmetry! we can do pointer -= integer,
    // but not integer -= pointer
    if( Operation->RightOperand->ReturnedType->Type() == DataTypes::Pointer )
    {
        RaiseError( Operation->Location, "incompatible types for operator -=" );
        return;
    }
    
    // now just check the non-assignment version
    CheckSubtraction( Operation );
}

// -----------------------------------------------------------------------------

void CheckProductAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckProduct( Operation );
}

// -----------------------------------------------------------------------------

void CheckDivisionAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckDivision( Operation );
}

// -----------------------------------------------------------------------------

void CheckModulusAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckModulus( Operation );
}

// -----------------------------------------------------------------------------

void CheckBitwiseAndAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckBitwiseAnd( Operation );
}

// -----------------------------------------------------------------------------

void CheckBitwiseOrAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckBitwiseOr( Operation );
}

// -----------------------------------------------------------------------------

void CheckBitwiseXorAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckBitwiseXor( Operation );
}

// -----------------------------------------------------------------------------

void CheckShiftLeftAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckShiftLeft( Operation );
}

// -----------------------------------------------------------------------------

void CheckShiftRightAssignment( BinaryOperationNode* Operation )
{
    // for all assignments, left operand has to be a variable
    if( !Operation->LeftOperand->HasMemoryPlacement() )
    {
        RaiseError( Operation->Location, "assignment destination must be have a memory address" );
        return;
    }
    
    // now just check the non-assignment version
    CheckShiftRight( Operation );
}
