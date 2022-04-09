// *****************************************************************************
    // start include guard
    #ifndef VECTOR2D_HPP
    #define VECTOR2D_HPP
// *****************************************************************************


// =============================================================================
//     2D VECTOR CLASS
// =============================================================================


class Vector2D
{
    public:

        float x, y;
    
    public:
        
        // class constructors
        Vector2D();
        Vector2D( float _x, float _y );
        Vector2D( const Vector2D& Copied );
        
        // vector module and angle
        float Module () const;                       // vector size as euclidean 2D distance
        float Module2() const;                       // module squared (faster, useful for comparisons)
        float Angle  () const;                       // vector angle with x axis (in radians)
        
        // vector module comparisons
        bool  operator>=( const Vector2D& V2 ) const;
        bool  operator<=( const Vector2D& V2 ) const;
        bool  operator> ( const Vector2D& V2 ) const;
        bool  operator< ( const Vector2D& V2 ) const;
    
        // associated vectors
        Vector2D Unit() const;          // unit vector in the same direction
        Vector2D RightNormal() const;   // right normal vector
        Vector2D LeftNormal () const;   // left normal vector
    
        // arithmetic operations with vectors
        float     operator* ( const Vector2D& V2 ) const;   // dot product
        Vector2D  operator+ ( const Vector2D& V2 ) const;
        Vector2D  operator- ( const Vector2D& V2 ) const;
        Vector2D& operator+=( const Vector2D& V2 );
        Vector2D& operator-=( const Vector2D& V2 );
        
        // arithmetic operations with scalars (double)
        Vector2D  operator* ( double Number ) const;
        Vector2D  operator/ ( double Number ) const;
        Vector2D& operator*=( double Number );
        Vector2D& operator/=( double Number );
        
        // arithmetic operations with scalars (float)
        Vector2D  operator* ( float Number ) const;
        Vector2D  operator/ ( float Number ) const;
        Vector2D& operator*=( float Number );
        Vector2D& operator/=( float Number );
        
        // arithmetic operations with scalars (int)
        Vector2D  operator* ( int Number ) const;
        Vector2D  operator/ ( int Number ) const;
        Vector2D& operator*=( int Number );
        Vector2D& operator/=( int Number );
        
        // unary operators
        Vector2D operator-( void ) const;
        operator bool () const;      // bad conversions if operator == is not defined
        bool operator!() const;
        
        // other operators
        bool      operator==( const Vector2D& V2 ) const;
        bool      operator!=( const Vector2D& V2 ) const;
        Vector2D& operator= ( const Vector2D& V2 );
};


// =============================================================================
//     VECTOR 2D: INLINE METHODS
// =============================================================================


inline bool Vector2D::operator==( const Vector2D& V2 ) const
{
    return ( (x == V2.x) && (y == V2.y) );
}

// -----------------------------------------------------------------------------

inline bool Vector2D::operator!=( const Vector2D& V2 ) const
{
    return ( (x != V2.x) || (y != V2.y) );
}

// -----------------------------------------------------------------------------

inline Vector2D Vector2D::operator-( void ) const
{
    return Vector2D( -x, -y );
}

// -----------------------------------------------------------------------------

inline Vector2D::operator bool() const
{
    return ( x || y );
}

// -----------------------------------------------------------------------------

inline bool Vector2D::operator!() const
{
    return !( x || y );
}

// -----------------------------------------------------------------------------

inline Vector2D& Vector2D::operator=( const Vector2D &V2 )
{
    x = V2.x;
    y = V2.y;
    
    return *this;
}


// =============================================================================
//     HELPER FUNCTIONS TO TREAT 2D VECTORS
// =============================================================================


// function for quick "literal-like" 2D vector declaration
inline Vector2D V( float x, float y )
{
    return Vector2D( x, y );
}

// -----------------------------------------------------------------------------

// returns what would be the Z component of the resulting 3D vector
float VectorProduct( const Vector2D& V1, const Vector2D& V2 );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
