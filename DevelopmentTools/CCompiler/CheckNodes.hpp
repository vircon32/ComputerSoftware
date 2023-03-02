// *****************************************************************************
    // start include guard
    #ifndef CHECKNODES_HPP
    #define CHECKNODES_HPP
    
    // include project headers
    #include "CNodes.hpp"
// *****************************************************************************


// =============================================================================
//      GENERAL NODE CHECKING FUNCTIONS
// =============================================================================


void CheckExpression( ExpressionNode* Expression );
void CheckAssignmentTypes( SourceLocation Location, DataType* LeftType, ExpressionNode* RightValue );


// =============================================================================
//      CHECK FUNCTIONS FOR EXPRESSION NODES
// =============================================================================


void CheckExpressionAtom( ExpressionAtomNode* Atom );
void CheckFunctionCall( FunctionCallNode* FunctionCall );
void CheckArrayAccess( ArrayAccessNode* ArrayAccess );
void CheckUnaryOperation( UnaryOperationNode* Operation );
void CheckBinaryOperation( BinaryOperationNode* Operation );
void CheckMemberAccess( MemberAccessNode* MemberAccess );
void CheckPointedMemberAccess( PointedMemberAccessNode* PointedMemberAccess );
void CheckSizeOf( SizeOfNode* SizeOf );
void CheckTypeConversion( TypeConversionNode* TypeConversion );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
