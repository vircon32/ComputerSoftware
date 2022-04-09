// *****************************************************************************
    // include project headers
    #include "Operators.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <map>          // [ C++ STL ] Maps
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      OPERATORS: CONVERSION TO STRING
// =============================================================================


const map< UnaryOperators, string > UnaryOperatorNames =
{
    { UnaryOperators::PlusSign,      "PlusSign"      },
    { UnaryOperators::MinusSign,     "MinusSign"     },
    { UnaryOperators::PreIncrement,  "PreIncrement"  },
    { UnaryOperators::PreDecrement,  "PreDecrement"  },
    { UnaryOperators::PostIncrement, "PostIncrement" },
    { UnaryOperators::PostDecrement, "PostDecrement" },
    { UnaryOperators::LogicalNot,    "LogicalNot"    },
    { UnaryOperators::BitwiseNot,    "BitwiseNot"    },
    { UnaryOperators::Reference,     "Reference"     },
    { UnaryOperators::Dereference,   "Dereference"   }
};

// -----------------------------------------------------------------------------

const map< BinaryOperators, string > BinaryOperatorNames =
{
    { BinaryOperators::Addition,              "Addition"              },
    { BinaryOperators::Subtraction,           "Subtraction"           },
    { BinaryOperators::Division,              "Division"              },
    { BinaryOperators::Product,               "Product"               },
    { BinaryOperators::Modulus,               "Modulus"               },
    { BinaryOperators::Equal,                 "Equal"                 },
    { BinaryOperators::NotEqual,              "NotEqual"              },
    { BinaryOperators::LessThan,              "LessThan"              },
    { BinaryOperators::LessOrEqual,           "LessOrEqual"           },
    { BinaryOperators::GreaterThan,           "GreaterThan"           },
    { BinaryOperators::GreaterOrEqual,        "GreaterOrEqual"        },
    { BinaryOperators::LogicalOr,             "LogicalOr"             },
    { BinaryOperators::LogicalAnd,            "LogicalAnd"            },
    { BinaryOperators::BitwiseOr,             "BitwiseOr"             },
    { BinaryOperators::BitwiseAnd,            "BitwiseAnd"            },
    { BinaryOperators::BitwiseXor,            "BitwiseXor"            },
    { BinaryOperators::ShiftLeft,             "ShiftLeft"             },
    { BinaryOperators::ShiftRight,            "ShiftRight"            },
    { BinaryOperators::MemberAccess,          "MemberAccess"          },
    { BinaryOperators::PointedMemberAccess,   "PointedMemberAccess"   },
    { BinaryOperators::Assignment,            "Assignment"            },
    { BinaryOperators::AdditionAssignment,    "AdditionAssignment"    },
    { BinaryOperators::SubtractionAssignment, "SubtractionAssignment" },
    { BinaryOperators::ProductAssignment,     "ProductAssignment"     },
    { BinaryOperators::DivisionAssignment,    "DivisionAssignment"    },
    { BinaryOperators::ModulusAssignment,     "ModulusAssignment"     },
    { BinaryOperators::BitwiseAndAssignment,  "BitwiseAndAssignment"  },
    { BinaryOperators::BitwiseOrAssignment,   "BitwiseOrAssignment"   },
    { BinaryOperators::BitwiseXorAssignment,  "BitwiseXorAssignment"  },
    { BinaryOperators::ShiftLeftAssignment,   "ShiftLeftAssignment"   },
    { BinaryOperators::ShiftRightAssignment,  "ShiftRightAssignment"  }
};

// -----------------------------------------------------------------------------

string UnaryOperatorToString( UnaryOperators Which )
{
    // just search in the map
    auto MapPair = UnaryOperatorNames.find( Which );
    if( MapPair != UnaryOperatorNames.end() )
      return MapPair->second;
    
    return "???";
}

// -----------------------------------------------------------------------------

string BinaryOperatorToString( BinaryOperators Which )
{
    // just search in the map
    auto MapPair = BinaryOperatorNames.find( Which );
    if( MapPair != BinaryOperatorNames.end() )
      return MapPair->second;
    
    return "???";
}

// -----------------------------------------------------------------------------

UnaryOperators TokenToUnaryOperator( OperatorToken* Token )
{
    switch( Token->Which )
    {
        case OperatorTypes::Plus:        return UnaryOperators::PlusSign;
        case OperatorTypes::Minus:       return UnaryOperators::MinusSign;
        case OperatorTypes::Exclamation: return UnaryOperators::LogicalNot;
        case OperatorTypes::Tilde:       return UnaryOperators::BitwiseNot;
        case OperatorTypes::DoublePlus:  return UnaryOperators::PreIncrement;
        case OperatorTypes::DoubleMinus: return UnaryOperators::PreDecrement;
        case OperatorTypes::Ampersand:   return UnaryOperators::Reference;
        case OperatorTypes::Asterisk:    return UnaryOperators::Dereference;
        
        default: break;
    }
    
    // operator not valid
    RaiseFatalError( Token->Location, "token is not a valid unary operator" );
}

// -----------------------------------------------------------------------------

BinaryOperators TokenToBinaryOperator( OperatorToken* Token )
{
    switch( Token->Which )
    {
        case OperatorTypes::Plus:               return BinaryOperators::Addition;
        case OperatorTypes::Minus:              return BinaryOperators::Subtraction;
        case OperatorTypes::Asterisk:           return BinaryOperators::Product;
        case OperatorTypes::Slash:              return BinaryOperators::Division;
        case OperatorTypes::Percent:            return BinaryOperators::Modulus;
        case OperatorTypes::DoubleEqual:        return BinaryOperators::Equal;
        case OperatorTypes::ExclamationEqual:   return BinaryOperators::NotEqual;
        case OperatorTypes::Less:               return BinaryOperators::LessThan;
        case OperatorTypes::LessEqual:          return BinaryOperators::LessOrEqual;
        case OperatorTypes::Greater:            return BinaryOperators::GreaterThan;
        case OperatorTypes::GreaterEqual:       return BinaryOperators::GreaterOrEqual;
        case OperatorTypes::DoubleVerticalBar:  return BinaryOperators::LogicalOr;
        case OperatorTypes::DoubleAmpersand:    return BinaryOperators::LogicalAnd;
        case OperatorTypes::VerticalBar:        return BinaryOperators::BitwiseOr;
        case OperatorTypes::Ampersand:          return BinaryOperators::BitwiseAnd;
        case OperatorTypes::Circumflex:         return BinaryOperators::BitwiseXor;
        case OperatorTypes::DoubleLess:         return BinaryOperators::ShiftLeft;
        case OperatorTypes::DoubleGreater:      return BinaryOperators::ShiftRight;
        case OperatorTypes::Dot:                return BinaryOperators::MemberAccess;
        case OperatorTypes::MinusGreater:       return BinaryOperators::PointedMemberAccess;
        case OperatorTypes::Equal:              return BinaryOperators::Assignment;
        case OperatorTypes::PlusEqual:          return BinaryOperators::AdditionAssignment;
        case OperatorTypes::MinusEqual:         return BinaryOperators::SubtractionAssignment;
        case OperatorTypes::AsteriskEqual:      return BinaryOperators::ProductAssignment;
        case OperatorTypes::SlashEqual:         return BinaryOperators::DivisionAssignment;
        case OperatorTypes::PercentEqual:       return BinaryOperators::ModulusAssignment;
        case OperatorTypes::AmpersandEqual:     return BinaryOperators::BitwiseAndAssignment;
        case OperatorTypes::VerticalBarEqual:   return BinaryOperators::BitwiseOrAssignment;
        case OperatorTypes::CircumflexEqual:    return BinaryOperators::BitwiseXorAssignment;
        case OperatorTypes::DoubleLessEqual:    return BinaryOperators::ShiftLeftAssignment;
        case OperatorTypes::DoubleGreaterEqual: return BinaryOperators::ShiftRightAssignment;
        
        default: break;
    }
    
    // operator not valid
    RaiseFatalError( Token->Location, "token is not a valid binary operator" );
}


// =============================================================================
//      BINARY OPERATOR PRECEDENCE AND ASSOCIATIVITY
// =============================================================================


// only needed for binary operators
int BinaryOperatorPrecedenceLevel( BinaryOperators Which )
{
    switch( Which )
    {
        case BinaryOperators::MemberAccess:
        case BinaryOperators::PointedMemberAccess:
            return 12;
        
        case BinaryOperators::Product:
        case BinaryOperators::Division:
        case BinaryOperators::Modulus:
            return 11;
        
        case BinaryOperators::Addition:
        case BinaryOperators::Subtraction:
            return 10;
        
        case BinaryOperators::ShiftLeft:
        case BinaryOperators::ShiftRight:
            return 9;
        
        case BinaryOperators::GreaterThan:
        case BinaryOperators::GreaterOrEqual:
        case BinaryOperators::LessThan:
        case BinaryOperators::LessOrEqual:
            return 8;
        
        case BinaryOperators::Equal:
        case BinaryOperators::NotEqual:
            return 7;
        
        case BinaryOperators::BitwiseAnd:
            return 6;
        
        case BinaryOperators::BitwiseXor:
            return 5;
        
        case BinaryOperators::BitwiseOr:
            return 4;
        
        case BinaryOperators::LogicalAnd:
            return 3;
        
        case BinaryOperators::LogicalOr:
            return 2;
        
        case BinaryOperators::Assignment:
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
            return 1;
        
        // this comparison is only for binary operators!
        default:
            return 0;
    }
}

// -----------------------------------------------------------------------------

// Only needed for binary operators.
// Left associative means:  A op B op C is taken as (A op B) op C
// Right associative means: A op B op C is taken as A op (B op C)
bool IsBinaryOperatorRightAssociative( BinaryOperators Which )
{
    switch( Which )
    {
        case BinaryOperators::Assignment:
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
            return true;
            
        default:
            return false;
    }
}

// -----------------------------------------------------------------------------

// LeftOp was parsed before RightOp
// (so RightOp is the "new operator")
bool NewBinaryOperatorHasPrecedence( BinaryOperators LeftOp, BinaryOperators RightOp )
{
    int PrecedenceLeftOp  = BinaryOperatorPrecedenceLevel( LeftOp );
    int PrecedenceRightOp = BinaryOperatorPrecedenceLevel( RightOp );
    
    // on equal precedence, take into account associativity
    // (for a right-to-left operator, new operations are
    // added with MORE priority (i.e. below in the tree))
    if( PrecedenceLeftOp == PrecedenceRightOp )
      return IsBinaryOperatorRightAssociative( RightOp );
    
    // in other cases, use precedences only
    return (PrecedenceRightOp > PrecedenceLeftOp);
}
