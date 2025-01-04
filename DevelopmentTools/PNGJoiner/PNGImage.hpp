// *****************************************************************************
    // start include guard
    #ifndef PNGIMAGE_HPP
    #define PNGIMAGE_HPP
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <list>         // [ C++ STL ] Lists
    #include <stdint.h>     // [ ANSI C ] Standard integers
// *****************************************************************************


// =============================================================================
//      PNG IMAGE CLASS
// =============================================================================


class PNGImage
{
    public:
    
        // basic image info
        std::string Name;
        int Width, Height;
        
        // configuration for matrices
        // (i.e. subimages laid out in a grid)
        int TilesX, TilesY;
        int TilesGap;        // same gap in X and Y; we assume no external border
        
        // pixel data
        uint8_t** RowPixels;
        
    public:
    
        // auxiliary functions
        void DecomposeFileName( const std::string& PNGFilePath );
        
    public:
        
        // instance handling
        PNGImage();
        PNGImage( const PNGImage& Copied );
       ~PNGImage();
        
        // file I/O
        void LoadFromFile( const std::string& PNGFilePath );
        void SaveToFile( const std::string& PNGFilePath ) const;
        
        // composing
        void CreateEmpty( int NewWidth, int NewHeight );
        void CopySubImage( const PNGImage& SubImage, int LeftX, int TopY );
        
        // queries for padded dimensions; these add
        // the gap at bottom-right to ensure separation
        int PaddedWidth() const;
        int PaddedHeight() const;
        int PaddedArea() const;
};


// =============================================================================
//      SORTING IMAGES
// =============================================================================


bool operator<( const PNGImage& Image1, const PNGImage& Image2 );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
