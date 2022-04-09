// *****************************************************************************
    // start include guard
    #ifndef CHECKBINARYOPERATIONS_HPP
    #define CHECKBINARYOPERATIONS_HPP
    
    // include project headers
    #include "CNodes.hpp"
// *****************************************************************************


// =============================================================================
//      SEPARATE CHECKS FOR EACH BINARY OPERATOR
// =============================================================================


void CheckAddition             ( BinaryOperationNode* Operation );
void CheckSubtraction          ( BinaryOperationNode* Operation );
void CheckDivision             ( BinaryOperationNode* Operation );
void CheckProduct              ( BinaryOperationNode* Operation );
void CheckModulus              ( BinaryOperationNode* Operation );
void CheckEqual                ( BinaryOperationNode* Operation );
void CheckNotEqual             ( BinaryOperationNode* Operation );
void CheckLessThan             ( BinaryOperationNode* Operation );
void CheckLessOrEqual          ( BinaryOperationNode* Operation );
void CheckGreaterThan          ( BinaryOperationNode* Operation );
void CheckGreaterOrEqual       ( BinaryOperationNode* Operation );
void CheckLogicalOr            ( BinaryOperationNode* Operation );
void CheckLogicalAnd           ( BinaryOperationNode* Operation );
void CheckBitwiseOr            ( BinaryOperationNode* Operation );
void CheckBitwiseAnd           ( BinaryOperationNode* Operation );
void CheckBitwiseXor           ( BinaryOperationNode* Operation );
void CheckShiftLeft            ( BinaryOperationNode* Operation );
void CheckShiftRight           ( BinaryOperationNode* Operation );
void CheckAssignment           ( BinaryOperationNode* Operation );
void CheckAdditionAssignment   ( BinaryOperationNode* Operation );
void CheckSubtractionAssignment( BinaryOperationNode* Operation );
void CheckProductAssignment    ( BinaryOperationNode* Operation );
void CheckDivisionAssignment   ( BinaryOperationNode* Operation );
void CheckModulusAssignment    ( BinaryOperationNode* Operation );
void CheckBitwiseAndAssignment ( BinaryOperationNode* Operation );
void CheckBitwiseOrAssignment  ( BinaryOperationNode* Operation );
void CheckBitwiseXorAssignment ( BinaryOperationNode* Operation );
void CheckShiftLeftAssignment  ( BinaryOperationNode* Operation );
void CheckShiftRightAssignment ( BinaryOperationNode* Operation );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
