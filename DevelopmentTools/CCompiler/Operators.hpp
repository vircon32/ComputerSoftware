// *****************************************************************************
    // start include guard
    #ifndef OPERATORS_HPP
    #define OPERATORS_HPP
    
    // include project headers
    #include "CTokens.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      UNARY AND BINARY OPERATORS
// =============================================================================


enum class UnaryOperators
{
    PlusSign,
    MinusSign,
    PreIncrement,
    PreDecrement,
    PostIncrement,
    PostDecrement,
    LogicalNot,
    BitwiseNot,
    Reference,
    Dereference
};

// -----------------------------------------------------------------------------

enum class BinaryOperators
{
    Addition,
    Subtraction,
    Division,
    Product,
    Modulus,
    Equal,
    NotEqual,
    LessThan,
    LessOrEqual,
    GreaterThan,
    GreaterOrEqual,
    LogicalOr,
    LogicalAnd,
    BitwiseOr,
    BitwiseAnd,
    BitwiseXor,
    ShiftLeft,
    ShiftRight,
    MemberAccess,
    PointedMemberAccess,
    Assignment,
    AdditionAssignment,
    SubtractionAssignment,
    ProductAssignment,
    DivisionAssignment,
    ModulusAssignment,
    BitwiseAndAssignment,
    BitwiseOrAssignment,
    BitwiseXorAssignment,
    ShiftLeftAssignment,
    ShiftRightAssignment
};

// -----------------------------------------------------------------------------

// conversion of node operators to string
std::string UnaryOperatorToString( UnaryOperators Which );
std::string BinaryOperatorToString( BinaryOperators Which );

// conversion of token operators to node operators
UnaryOperators TokenToUnaryOperator( OperatorToken* Operator );
BinaryOperators TokenToBinaryOperator( OperatorToken* Operator );


// =============================================================================
//      BINARY OPERATOR PRECEDENCE AND ASSOCIATIVITY
// =============================================================================


int BinaryOperatorPrecedenceLevel( BinaryOperators Which );
bool IsBinaryOperatorRightAssociative( BinaryOperators Which );

// takes into account both of the above
// (called when building binary op trees)
bool NewBinaryOperatorHasPrecedence( BinaryOperators LeftOp, BinaryOperators RightOp );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

