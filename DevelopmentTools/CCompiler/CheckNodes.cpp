// *****************************************************************************
    // include project files
    #include "CheckNodes.hpp"
    #include "CheckUnaryOperations.hpp"
    #include "CheckBinaryOperations.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GENERAL NODE CHECKING FUNCTIONS
// =============================================================================


void CheckExpression( ExpressionNode* Expression )
{
    switch( Expression->Type() )
    {
        case CNodeTypes::ExpressionAtom:
            CheckExpressionAtom( (ExpressionAtomNode*)Expression );
            return;
        case CNodeTypes::FunctionCall:
            CheckFunctionCall( (FunctionCallNode*)Expression );
            return;
        case CNodeTypes::ArrayAccess:
            CheckArrayAccess( (ArrayAccessNode*)Expression );
            return;
        case CNodeTypes::UnaryOperation:
            CheckUnaryOperation( (UnaryOperationNode*)Expression );
            return;
        case CNodeTypes::BinaryOperation:
            CheckBinaryOperation( (BinaryOperationNode*)Expression );
            return;
        case CNodeTypes::EnclosedExpression:
            CheckExpression( ((EnclosedExpressionNode*)Expression)->InternalExpression );
            return;
        case CNodeTypes::MemberAccess:
            CheckMemberAccess( (MemberAccessNode*)Expression );
            return;
        case CNodeTypes::PointedMemberAccess:
            CheckPointedMemberAccess( (PointedMemberAccessNode*)Expression );
            return;
        case CNodeTypes::SizeOf:
            CheckSizeOf( (SizeOfNode*)Expression );
            return;
        case CNodeTypes::LiteralString:
            // (there is nothing to check here)
            return;
        case CNodeTypes::TypeConversion:
            CheckTypeConversion( (TypeConversionNode*)Expression );
            return;
        default:
            RaiseFatalError( Expression->Location, "invalid expression type" );
    }
}

// -----------------------------------------------------------------------------

void CheckAssignmentTypes( SourceLocation Location, DataType* LeftType, ExpressionNode* RightValue )
{
    // no void types are allowed
    DataType* RightType = RightValue->ReturnedType;
    
    if( RightType->Type() == DataTypes::Void )
    {
        RaiseError( Location, "cannot assign an expression returning no value" );
        return;
    }
    
    // arrays cannot be assigned a value (only their elements can)
    if( LeftType->Type() == DataTypes::Array )
    {
        RaiseError( Location, "cannot assign a value to an array" );
        return;
    }
    
    // case 1: they are the same type
    bool Case1Met = AreEqual( LeftType, RightType );
    
    // case 2: all primitives can be converted to one another
    bool Case2Met = (LeftType->Type() == DataTypes::Primitive)
                 && (RightType->Type() == DataTypes::Primitive);
    
    // case 3: enumerations can be assigned to all primitives
    // (but not the other way around: enums are more restrictive)
    bool Case3Met = (LeftType->Type() == DataTypes::Primitive)
                 && (RightType->Type() == DataTypes::Enumeration);
    
    // case 4: all pointers can be assigned NULL i.e. integer -1
    bool Case4Met = (LeftType->Type() == DataTypes::Pointer)
                 &&  TypeIsThisPrimitive( RightType, PrimitiveTypes::Int )
                 &&  RightValue->IsStatic()
                 && (RightValue->GetStaticValue().Word.AsInteger == -1);
    
    // case 5: all pointers can be assigned to void pointer
    bool Case5Met = (LeftType->Type() == DataTypes::Pointer)
                 && (RightType->Type() == DataTypes::Pointer)
                 && (((PointerType*)LeftType)->BaseType->Type() == DataTypes::Void);
    
    // case 6: arrays can be assigned to pointers to the same
    // base type (valid because of array decay into a pointer)
    bool Case6Met = (LeftType->Type() == DataTypes::Pointer)
                 && (RightType->Type() == DataTypes::Array)
                 &&  AreEqual( ((PointerType*)LeftType)->BaseType, ((ArrayType*)RightType)->BaseType );
    
    // case 6: arrays can be assigned to pointers to void
    // (valid because of array decay into a pointer)
    bool Case7Met = (LeftType->Type() == DataTypes::Pointer)
                 && (RightType->Type() == DataTypes::Array)
                 && (((PointerType*)LeftType)->BaseType->Type() == DataTypes::Void);
    
    if( !Case1Met && !Case2Met && !Case3Met && !Case4Met && !Case5Met && !Case6Met && !Case7Met )
      RaiseError( Location, "types are not compatible: cannot assign " + RightType->ToString() + " to " + LeftType->ToString() );
}


// =============================================================================
//      CHECK FUNCTIONS FOR EXPRESSION NODES
// =============================================================================


void CheckExpressionAtom( ExpressionAtomNode* Atom )
{
    // check that any referenced variables have been resolved
    if( Atom->AtomType == AtomTypes::Variable )
      if( !Atom->ResolvedVariable )
        RaiseFatalError( Atom->Location, string("variable \"") + Atom->IdentifierName + "\" has not been resolved" );
    
    // check that any referenced enumeration values have been resolved
    if( Atom->AtomType == AtomTypes::EnumValue )
      if( !Atom->ResolvedEnumValue )
        RaiseFatalError( Atom->Location, string("enumeration value \"") + Atom->IdentifierName + "\" has not been resolved" );
    
    // other atom types are always correct
}

// -----------------------------------------------------------------------------

void CheckFunctionCall( FunctionCallNode* FunctionCall )
{
    // first, recursively check the parameters
    for( ExpressionNode* Parameter: FunctionCall->Parameters )
      CheckExpression( Parameter );
    
    // check that its function has been resolved
    if( !FunctionCall->ResolvedFunction )
      RaiseFatalError( FunctionCall->Location, string("function \"") + FunctionCall->FunctionName + "\" has not been resolved" );
    
    FunctionNode* Function = FunctionCall->ResolvedFunction;
    
    // check the number of parameters
    int ExpectedParameters = Function->Arguments.size();
    int ReceivedParameters = FunctionCall->Parameters.size();
    
    if( ReceivedParameters != ExpectedParameters )
    {
        string Message = "function '" + FunctionCall->FunctionName + "' expects ";
        Message += to_string( ExpectedParameters ) + " parameters, ";
        Message += to_string( ReceivedParameters ) + " were received";
        
        RaiseError( FunctionCall->Location, Message );
        return;
    }
    
    // check type compatibility for each parameter
    auto ParameterIterator = FunctionCall->Parameters.begin();
    auto ArgumentIterator = Function->Arguments.begin();
    
    while( ParameterIterator != FunctionCall->Parameters.end() )
    {
        CheckAssignmentTypes( (*ParameterIterator)->Location, (*ArgumentIterator)->DeclaredType, *ParameterIterator );
        
        ParameterIterator++;
        ArgumentIterator++;
    }
}

// -----------------------------------------------------------------------------

void CheckArrayAccess( ArrayAccessNode* ArrayAccess )
{
    // first, recursively check the operands
    CheckExpression( ArrayAccess->ArrayOperand );
    CheckExpression( ArrayAccess->IndexOperand );
    
    // array operand needs to be of type array or pointer
    DataType* ArrayType = ArrayAccess->ArrayOperand->ReturnedType;
    
    if( ArrayType->Type() != DataTypes::Array && ArrayType->Type() != DataTypes::Pointer )
      RaiseError( ArrayAccess->Location, "array access can only be done from an array or pointer" );
    
    // index operand needs to be an integer
    // (careful: floats are not allowed to
    // be converted, but bool is taken as int)
    DataType* IndexType = ArrayAccess->IndexOperand->ReturnedType;
    
    if( !TypeIsIntegral( IndexType ) )
      RaiseError( ArrayAccess->Location, "array index must be integer" );
}

// -----------------------------------------------------------------------------

void CheckUnaryOperation( UnaryOperationNode* Operation )
{
    // first, recursively check the operand
    CheckExpression( Operation->Operand );
    
    // basic check: operand cannot be of type void
    DataType* OperandType = Operation->Operand->ReturnedType;
    
    if( OperandType->Type() == DataTypes::Void )
    {
        RaiseError( Operation->Location, "unary operation uses an expression that returns no value" );
        return;
    }
    
    // now choose the specific check function
    switch( Operation->Operator )
    {
        case UnaryOperators::PlusSign:
            CheckPlusSign( Operation );
            return;
        case UnaryOperators::MinusSign:
            CheckMinusSign( Operation );
            return;
        case UnaryOperators::PreIncrement:
        case UnaryOperators::PostIncrement:
            CheckIncrement( Operation );
            return;
        case UnaryOperators::PreDecrement:
        case UnaryOperators::PostDecrement:
            CheckDecrement( Operation );
            return;
        case UnaryOperators::LogicalNot:
            CheckLogicalNot( Operation );
            return;
        case UnaryOperators::BitwiseNot:
            CheckBitwiseNot( Operation );
            return;
        case UnaryOperators::Reference:
            CheckReference( Operation );
            return;
        case UnaryOperators::Dereference:
            CheckDereference( Operation );
            return;
            
        default:
            RaiseFatalError( Operation->Location, "cannot check invalid unary operator" );
    }
}

// -----------------------------------------------------------------------------

void CheckBinaryOperation( BinaryOperationNode* Operation )
{
    // first, recursively check the operands
    CheckExpression( Operation->LeftOperand );
    CheckExpression( Operation->RightOperand );
    
    // basic check: no operand cannot be of type void
    DataType* LeftType = Operation->LeftOperand->ReturnedType;
    DataType* RightType = Operation->RightOperand->ReturnedType;
    
    if( LeftType->Type() == DataTypes::Void || RightType->Type() == DataTypes::Void )
    {
        RaiseError( Operation->Location, "binary operation uses an expression that returns no value" );
        return;
    }
    
    // now choose the specific check function
    switch( Operation->Operator )
    {
        case BinaryOperators::Addition:
            CheckAddition( Operation );
            return;
        case BinaryOperators::Subtraction:
            CheckSubtraction( Operation );
            return;
        case BinaryOperators::Division:
            CheckDivision( Operation );
            return;
        case BinaryOperators::Product:
            CheckProduct( Operation );
            return;
        case BinaryOperators::Modulus:
            CheckModulus( Operation );
            return;
        case BinaryOperators::Equal:
            CheckEqual( Operation );
            return;
        case BinaryOperators::NotEqual:
            CheckNotEqual( Operation );
            return;
        case BinaryOperators::LessThan:
            CheckLessThan( Operation );
            return;
        case BinaryOperators::LessOrEqual:
            CheckLessOrEqual( Operation );
            return;
        case BinaryOperators::GreaterThan:
            CheckGreaterThan( Operation );
            return;
        case BinaryOperators::GreaterOrEqual:
            CheckGreaterOrEqual( Operation );
            return;
        case BinaryOperators::LogicalOr:
            CheckLogicalOr( Operation );
            return;
        case BinaryOperators::LogicalAnd:
            CheckLogicalAnd( Operation );
            return;
        case BinaryOperators::BitwiseOr:
            CheckBitwiseOr( Operation );
            return;
        case BinaryOperators::BitwiseAnd:
            CheckBitwiseAnd( Operation );
            return;
        case BinaryOperators::BitwiseXor:
            CheckBitwiseXor( Operation );
            return;
        case BinaryOperators::ShiftLeft:
            CheckShiftLeft( Operation );
            return;
        case BinaryOperators::ShiftRight:
            CheckShiftRight( Operation );
            return;
        case BinaryOperators::Assignment:
            CheckAssignment( Operation );
            return;
        case BinaryOperators::AdditionAssignment:
            CheckAdditionAssignment( Operation );
            return;
        case BinaryOperators::SubtractionAssignment:
            CheckSubtractionAssignment( Operation );
            return;
        case BinaryOperators::ProductAssignment:
            CheckProductAssignment( Operation );
            return;
        case BinaryOperators::DivisionAssignment:
            CheckDivisionAssignment( Operation );
            return;
        case BinaryOperators::ModulusAssignment:
            CheckModulusAssignment( Operation );
            return;
        case BinaryOperators::BitwiseAndAssignment:
            CheckBitwiseAndAssignment( Operation );
            return;
        case BinaryOperators::BitwiseOrAssignment:
            CheckBitwiseOrAssignment( Operation );
            return;
        case BinaryOperators::BitwiseXorAssignment:
            CheckBitwiseXorAssignment( Operation );
            return;
        case BinaryOperators::ShiftLeftAssignment:
            CheckShiftLeftAssignment( Operation );
            return;
        case BinaryOperators::ShiftRightAssignment:
            CheckShiftRightAssignment( Operation );
            return;
        
        default:
            RaiseFatalError( Operation->Location, "cannot check invalid binary operator" );
    }
}

// -----------------------------------------------------------------------------

void CheckMemberAccess( MemberAccessNode* MemberAccess )
{
    // first, recursively check the operands
    CheckExpression( MemberAccess->GroupOperand );
    
    // no further checks needed
    // (already done on parsing)
}

// -----------------------------------------------------------------------------

void CheckPointedMemberAccess( PointedMemberAccessNode* PointedMemberAccess )
{
    // first, recursively check the operands
    CheckExpression( PointedMemberAccess->GroupOperand );
    
    // no further checks needed
    // (already done on parsing)
}

// -----------------------------------------------------------------------------

void CheckSizeOf( SizeOfNode* SizeOf )
{
    // first, recursively check the operands
    if( SizeOf->QueriedExpression )
      CheckExpression( SizeOf->QueriedExpression );
    
    // it is invalid to query for size void
    if( SizeOf->QueriedType )
      if( SizeOf->QueriedType->Type() == DataTypes::Void )
        RaiseError( SizeOf->Location, "the size of a void type is undefined" );
}

// -----------------------------------------------------------------------------

void CheckTypeConversion( TypeConversionNode* TypeConversion )
{
    // first, recursively check the operands
    CheckExpression( TypeConversion->ConvertedExpression );
    
    // obtain both types
    DataType* OriginalType = TypeConversion->ConvertedExpression->ReturnedType;
    DataType* RequestedType = TypeConversion->RequestedType;
    
    // original type must be valid
    if( OriginalType->SizeInWords() == 0 )
      RaiseError( TypeConversion->Location, "cannot convert an expression that returns type " + OriginalType->ToString() );
    
    // requested type must be valid
    if( RequestedType->SizeInWords() == 0 )
      RaiseError( TypeConversion->Location, "cannot convert expression to type " + RequestedType->ToString() );
    
    // we will allow any kind of conversions between:
    // primitives types, pointers and enumerations
    bool OriginalTypeAllowed =
    (
        OriginalType->Type() == DataTypes::Enumeration ||
        OriginalType->Type() == DataTypes::Primitive ||
        OriginalType->Type() == DataTypes::Pointer
    );
    
    bool RequestedTypeAllowed =
    (
        RequestedType->Type() == DataTypes::Enumeration ||
        RequestedType->Type() == DataTypes::Primitive ||
        RequestedType->Type() == DataTypes::Pointer
    );
    
    if( !OriginalTypeAllowed || !RequestedTypeAllowed )
      RaiseError( TypeConversion->Location, "cannot convert expression type from " + OriginalType->ToString() + " to " + RequestedType->ToString() );
    
    // for enums, we only allow conversions from
    // integral types (int, bool or other enum)
    else if( RequestedType->Type() == DataTypes::Enumeration )
    {
        if( !TypeIsIntegral( OriginalType ) )
          RaiseError( TypeConversion->Location, "can only convert to enumerations from integral types" );
    }
    
    // do not allow float <-> pointers conversions
    else
    {
        if( (TypeIsFloat( OriginalType ) && RequestedType->Type() == DataTypes::Pointer)
        ||  (TypeIsFloat( RequestedType ) && OriginalType->Type() == DataTypes::Pointer) )
          RaiseError( TypeConversion->Location, "cannot convert between float and pointer types" );
    }
}
