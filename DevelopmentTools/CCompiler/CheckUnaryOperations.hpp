// *****************************************************************************
    // start include guard
    #ifndef CHECKUNARYOPERATIONS_HPP
    #define CHECKUNARYOPERATIONS_HPP
    
    // include project headers
    #include "CNodes.hpp"
// *****************************************************************************


// =============================================================================
//      SEPARATE CHECKS FOR EACH UNARY OPERATOR
// =============================================================================


void CheckPlusSign    ( UnaryOperationNode* Operation );
void CheckMinusSign   ( UnaryOperationNode* Operation );
void CheckIncrement   ( UnaryOperationNode* Operation );
void CheckDecrement   ( UnaryOperationNode* Operation );
void CheckLogicalNot  ( UnaryOperationNode* Operation );
void CheckBitwiseNot  ( UnaryOperationNode* Operation );
void CheckReference   ( UnaryOperationNode* Operation );
void CheckDereference ( UnaryOperationNode* Operation );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
