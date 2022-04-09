// *****************************************************************************
    // include project headers
    #include "Vector2D.hpp"
    #include "Definitions.hpp"
    
    // include C/C++ headers
    #include <math.h>           // [ ANSI C ] Mathematics
    #include <stdexcept>        // [ C++ STL ] Exceptions
    
    using namespace std;
// *****************************************************************************
    

// =============================================================================
//      VECTOR 2D: CONSTRUCTORS
// =============================================================================


Vector2D::Vector2D()
// - - - - - -
:   x( 0 ),
    y( 0 )
// - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

Vector2D::Vector2D( float _x, float _y )
// - - - - - -
:   x( _x ),
    y( _y )
// - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

Vector2D::Vector2D( const Vector2D &Copied )
// - - - - - - - - - - -
:   x( Copied.x ),
    y( Copied.y )
// - - - - - - - - - - -
{
    // (do nothing)
}
    

// =============================================================================
//      VECTOR 2D: MODULE AND ANGLE
// =============================================================================


float Vector2D::Module() const
{
    return sqrt( x*x + y*y );
}

// -----------------------------------------------------------------------------

float Vector2D::Module2() const
{
    return ( x*x + y*y );
}

// -----------------------------------------------------------------------------

float Vector2D::Angle() const
{
    return CalculateAngle( x, y );
}


// =============================================================================
//      VECTOR 2D: VECTOR COMPARISONS
// =============================================================================


bool Vector2D::operator>=( const Vector2D& V2 ) const
{
    return ( Module2() >= V2.Module2() );
}

// -----------------------------------------------------------------------------

bool Vector2D::operator<=( const Vector2D& V2 ) const
{
    return ( Module2() <= V2.Module2() );
}

// -----------------------------------------------------------------------------

bool Vector2D::operator>( const Vector2D& V2 ) const
{
    return ( Module2() > V2.Module2() );
}

// -----------------------------------------------------------------------------

bool Vector2D::operator<( const Vector2D& V2 ) const
{
    return ( Module2() < V2.Module2() );
}


// =============================================================================
//      VECTOR 2D: ASSOCIATED VECTORS
// =============================================================================


Vector2D Vector2D::Unit() const
{
    if( !(x || y) )
      throw runtime_error( "Vector2D: Cannot make unit vector from null vector" );
     
    Vector2D Result = *this / Module();
    return Result;
}

// -----------------------------------------------------------------------------

Vector2D Vector2D::RightNormal() const
{
    return Vector2D( y, -x );
}

// -----------------------------------------------------------------------------

Vector2D Vector2D::LeftNormal() const
{
    return Vector2D( -y, x );
}


// =============================================================================
//      VECTOR 2D: ARTIHMETIC WITH VECTORS
// =============================================================================


float Vector2D::operator*( const Vector2D& V2 ) const
{
    return ( x * V2.x + y * V2.y );
}
    
// -----------------------------------------------------------------------------

Vector2D Vector2D::operator+( const Vector2D& V2 ) const
{
    return Vector2D( x + V2.x, y + V2.y );
}

// -----------------------------------------------------------------------------
    
Vector2D Vector2D::operator-( const Vector2D& V2 ) const
{
    return Vector2D( x - V2.x, y - V2.y );
}

// -----------------------------------------------------------------------------

Vector2D& Vector2D::operator+=( const Vector2D& V2 )
{
    x += V2.x;
    y += V2.y;
    
    return *this;
}

// -----------------------------------------------------------------------------

Vector2D& Vector2D::operator-=( const Vector2D& V2 )
{
    x -= V2.x;
    y -= V2.y;
    
    return *this;
}


// =============================================================================
//      VECTOR 2D: ARTIHMETIC WITH SCALARS (DOUBLE)
// =============================================================================


Vector2D Vector2D::operator*( double Number ) const
{
    return Vector2D( x * Number, y * Number );
}

// -----------------------------------------------------------------------------

Vector2D Vector2D::operator/( double Number ) const
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    return Vector2D( x / Number, y / Number );
}

// -----------------------------------------------------------------------------
    
Vector2D& Vector2D::operator*=( double Number )
{
    x *= Number;
    y *= Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

Vector2D& Vector2D::operator/=( double Number )
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    x /= Number;
    y /= Number;
    
    return *this;
}


// =============================================================================
//      VECTOR 2D: ARTIHMETIC WITH SCALARS (FLOAT)
// =============================================================================


Vector2D Vector2D::operator*( float Number ) const
{
    return Vector2D( x * Number, y * Number );
}

// -----------------------------------------------------------------------------

Vector2D Vector2D::operator/( float Number ) const
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    return Vector2D( x / Number, y / Number );
}

// -----------------------------------------------------------------------------
    
Vector2D& Vector2D::operator*=( float Number )
{
    x *= Number;
    y *= Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

Vector2D& Vector2D::operator/=( float Number )
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    x /= Number;
    y /= Number;
    
    return *this;
}


// =============================================================================
//      VECTOR 2D: ARTIHMETIC WITH SCALARS (INT)
// =============================================================================


Vector2D Vector2D::operator*( int Number ) const
{
    return Vector2D( x * Number, y * Number );
}

// -----------------------------------------------------------------------------

Vector2D Vector2D::operator/( int Number ) const
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    return Vector2D( x / Number, y / Number );
}

// -----------------------------------------------------------------------------
    
Vector2D& Vector2D::operator*=( int Number )
{
    x *= Number;
    y *= Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

Vector2D& Vector2D::operator/=( int Number )
{
    if( !Number )
      throw runtime_error( "Vector2D: Attempted division by zero" );
    
    x /= Number;
    y /= Number;
    
    return *this;
}


// =============================================================================
//     HELPER FUNCTIONS TO TREAT 2D VECTORS
// =============================================================================


float VectorProduct( const Vector2D& V1, const Vector2D& V2 )
{
    return (V1.x * V2.y) - (V1.y * V2.x);
}
