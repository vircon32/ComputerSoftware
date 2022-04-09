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
        default:
            RaiseFatalError( Expression->Location, "invalid expression type" );
    }
}

// -----------------------------------------------------------------------------

void CheckTypeConversion( SourceLocation Location, ExpressionNode* ProducedValue, DataType* NeededType )
{
    DataType* ProducedType = ProducedValue->ReturnedType;
    
    // no void types are allowed
    if( ProducedType->Type() == DataTypes::Void )
    {
        RaiseError( Location, "cannot assign an expression returning no value" );
        return;
    }
    
    // no conversion needed if types are equal
    if( AreEqual( ProducedType, NeededType ) )
      return;
    
    // arrays can be converted into pointers (i.e. array
    // decay) to either the same base type or to void
    if( ProducedType->Type() == DataTypes::Array )
      if( NeededType->Type() == DataTypes::Pointer )
      {
          DataType* PointerBaseType = ((PointerType*)NeededType)->BaseType;
          DataType* ArrayBaseType = ((ArrayType*)ProducedType)->BaseType;
          
          if( AreEqual( PointerBaseType, ArrayBaseType ) )
            return;
          
          if( PointerBaseType->Type() == DataTypes::Void )
            return;
      }
    
    // enumerations can be converted into all primitive types
    // (but not the other way around)
    if( ProducedType->Type() == DataTypes::Enumeration )
      if( NeededType->Type() == DataTypes::Primitive )
        return;
    
    // NULL, or its value -1, can be converted to any pointer
    if( NeededType->Type() == DataTypes::Pointer )
      if( TypeIsThisPrimitive( ProducedType, PrimitiveTypes::Int ) )
        if( ProducedValue->IsStatic() )
          if( ProducedValue->GetStaticValue().Word.AsInteger == -1 )
            return;
    
    // in other cases, different classes of types can never be converted
    if( ProducedType->Type() != NeededType->Type() )
    {
        RaiseError( Location, "cannot convert " + ProducedType->ToString() + " to " + NeededType->ToString() );
        return;
    }
    
    // primitives can always be converted
    if( ProducedType->Type() == DataTypes::Primitive )
      return;
    
    // allow conversion of any pointer to void pointer
    if( ProducedType->Type() == DataTypes::Pointer )
    {
        DataType* NeededBase = ((PointerType*)NeededType)->BaseType;
        
        if( NeededBase->Type() == DataTypes::Void )
          return;
    }
    
    // forbid conversion in any other case
    RaiseError( Location, "cannot convert " + ProducedType->ToString() + " to " + NeededType->ToString() );
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
        CheckTypeConversion( (*ParameterIterator)->Location, (*ParameterIterator), (*ArgumentIterator)->DeclaredType );
        
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
        RaiseError( SizeOf->Location, "the size of a void type is undefined"  );
}
