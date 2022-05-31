// *****************************************************************************
    // start include guard
    #ifndef COLORS_HPP
    #define COLORS_HPP
    
    // include project headers
    #include "Definitions.hpp"
// *****************************************************************************


// =============================================================================
//      RGB COLOR HANDLING
// =============================================================================


class Color
{
    public:
        
        uint8_t R,G,B,A;
    
    public:
        
        // default to opaque black
        Color()
        :  Color( 0, 0, 0, 255 )
        {}
        
        Color( uint8_t R_, uint8_t G_, uint8_t B_ )
        :  Color( R_, G_, B_, 255 )
        {}
        
        Color( const Color& Base, uint8_t A_ )
        :  Color( Base.R, Base.G, Base.B, A_ )
        {}
        
        // the only constructor that allows us to choose
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
void SetClearColor( const Color& C );


// =============================================================================
//      COLOR BLENDING MODES
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


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
