// *****************************************************************************
    // include project headers
    #include "Matrix4D.hpp"
    
    // include C/C++ headers
    #include <cstring>          // [ ANSI C ] Strings
// *****************************************************************************


// =============================================================================
//      MATRIX 4D: CLASS IMPLEMENTATION
// =============================================================================



Matrix4D::Matrix4D()
{
    LoadIdentity();
}

// -----------------------------------------------------------------------------

void Matrix4D::LoadIdentity()
{
    float* Component = &Components[0][0];
    
    for( int i = 0; i < (4*4); i++ )
      Component[ i ] = 0.0;
    
    for( int i = 0; i < 4; i++ )
      Components[ i ][ i ] = 1.0;
}

// -----------------------------------------------------------------------------

Matrix4D& Matrix4D::operator=( Matrix4D& M4 )
{
    memcpy( &Components[0][0], &M4.Components[0][0], 4*4*sizeof(float) );
    return *this;
}

// -----------------------------------------------------------------------------

Matrix4D& Matrix4D::operator*=( Matrix4D& M4 )
{
    Matrix4D ResultMatrix;
    
    for( int Column = 0; Column < 4; Column++ )
      for( int Row = 0; Row < 4; Row++ )
        {
            float* Result = &ResultMatrix.Components[ Row ][ Column ];
            *Result = 0.0;
            
            for( int i = 0; i < 4; i++ )
              *Result += M4.Components[ i ][ Column ] * Components[ Row ][ i ];
        }
    
    return operator=( ResultMatrix );
}
