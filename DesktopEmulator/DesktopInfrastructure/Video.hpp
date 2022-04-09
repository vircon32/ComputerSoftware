// *****************************************************************************
    // start include guard
    #ifndef VIDEO_HPP
    #define VIDEO_HPP
    
    // include project headers
    #include "Definitions.hpp"
    #include "Vector2D.hpp"
    
    // include C/C++ headers
    #include <vector>           // [ C++ STL ] Vectors
// *****************************************************************************


// =============================================================================
//      RGB COLOR HANDLING
// =============================================================================


class Color
{
    public:
        
        uint8_t R,G,B,A;
    
    public:
        
        // default to black
        Color()
        :  Color( 0, 0, 0, 255 )
        {}
        
        Color( uint8_t R_, uint8_t G_, uint8_t B_ )
        :  Color( R_, G_, B_, 255 )
        {}
        
        Color( const Color& Base, uint8_t A_ )
        :  Color( Base.R, Base.G, Base.B, A_ )
        {}
        
        // the only actual constructor
        Color( uint8_t R_, uint8_t G_, uint8_t B_, uint8_t A_ )
        {
            R = R_;
            G = G_;
            B = B_;
            A = A_;
        }
};

// -----------------------------------------------------------------------------

// now define some common colors
namespace Colors
{
    extern Color Black;
    extern Color White;
    extern Color Red;
    extern Color Green;
    extern Color Blue;
    extern Color Yellow;
    extern Color Magenta;
    extern Color Cyan;
    extern Color Grey;
    extern Color DarkGrey;
    extern Color LightGrey;
    extern Color Purple;
    extern Color Orange;
    extern Color Brown;
}

// -----------------------------------------------------------------------------

// color treatment functions
void SetColor( const Color& C );
void SetClearColor( const Color& C );


// =============================================================================
//      BLENDING MODES
// =============================================================================


enum class BlendingMode
{
    Null,         // renders nothing
    Solid,        // renders ignoring alpha
    Alpha,        // renders with alpha as transparency
    Add,          // adds colors (black is "transparent")
    Subtract,     // sutracts colors (white is "transparent")
    Multiply      // multiplies colors, with transparency
};

// -----------------------------------------------------------------------------

void SetBlendingMode( BlendingMode Mode );


// =============================================================================
//      DEFINITIONS FOR VERTICES
// =============================================================================


// point array primitives
typedef std::vector<Vector2D> V2Array;

// helper functions to define a 2D OpenGL vertex from a vector
void glVertexV2( const Vector2D& Vertex );
void glTexCoordV2( const Vector2D& TexturePosition );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
