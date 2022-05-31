// *****************************************************************************
    // start include guard
    #ifndef MATRIX4D_HPP
    #define MATRIX4D_HPP
// *****************************************************************************


// =============================================================================
//      CLASS FOR A CUATERNION MATRIX USABLE IN OPENGL SHADERS
// =============================================================================


class Matrix4D
{
    public:
    
        float Components[ 4 ][ 4 ];  // in order: [ Row ][ Column ]
        
    public:
        
        // instance handling
        Matrix4D();
        
        // basic operations
        void LoadIdentity();
        Matrix4D& operator=( Matrix4D& M4 );
        Matrix4D& operator*=( Matrix4D& M4 );    // left-side multiply: (*this) = M4 * (*this)
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
