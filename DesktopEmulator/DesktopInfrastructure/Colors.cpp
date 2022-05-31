// *****************************************************************************
    // include project headers
    #include "Colors.hpp"
    
    // include OpenGL headers
    #include <glad/glad.h>      // [ OpenGL ] GLAD Loader (already includes <GL/gl.h>)
// *****************************************************************************


// =============================================================================
//      RGB COLOR HANDLING
// =============================================================================


namespace Colors
{
    Color Black     (   0,   0,   0 );
    Color White     ( 255, 255, 255 );
    Color Red       ( 255,   0,   0 );
    Color Green     (   0, 255,   0 );
    Color Blue      (   0,   0, 255 );
    Color Yellow    ( 255, 255,   0 );
    Color Magenta   ( 255,   0, 255 );
    Color Cyan      (   0, 255, 255 );
    Color Grey      ( 127, 127, 127 );
    Color DarkGrey  (  63,  63,  63 );
    Color LightGrey ( 191, 191, 191 );
    Color Purple    ( 127,   0, 127 );
    Color Orange    ( 255, 127,   0 );
    Color Brown     ( 127,  63,   0 );
}

// -----------------------------------------------------------------------------

void SetClearColor( const Color& C )
{
    // alpha cannot be set in this case
    glClearColor( C.R/255.0, C.G/255.0, C.B/255.0, 1.0 );
}


// =============================================================================
//      COLOR BLENDING MODES
// =============================================================================


void SetBlendingMode( BlendingMode Mode )
{
    switch( Mode )
    {
        case BlendingMode::Null:       glBlendFunc( GL_ZERO     , GL_ONE                 ); break;
        case BlendingMode::Solid:      glBlendFunc( GL_ONE      , GL_ZERO                ); break;
        case BlendingMode::Alpha:      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); break;
        case BlendingMode::Add:        glBlendFunc( GL_SRC_ALPHA, GL_ONE                 ); break;
        case BlendingMode::Subtract:   glBlendFunc( GL_SRC_ALPHA, GL_ONE                 ); break;
        case BlendingMode::Multiply:   glBlendFunc( GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA ); break;
    }
    
    // substraction needs an extra tweak
    glBlendEquation( (Mode == BlendingMode::Subtract)? GL_FUNC_REVERSE_SUBTRACT : GL_FUNC_ADD );
}
