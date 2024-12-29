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
    
        std::string Name;
        int FirstTileID;
        int Width, Height;
        int TilesX, TilesY;
        int TilesGap;     // same in X and Y; no external frame
        
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
        
        // basic queries; padded dimensions are the gap at bottom-right
        int Area() const;
        int PaddedWidth() const;
        int PaddedHeight() const;
        int PaddedArea() const;
};


// =============================================================================
//      SORTING IMAGES
// =============================================================================


bool operator<( const PNGImage& Image1, const PNGImage& Image2 );
void AssignRegionIDs( std::list< PNGImage >& LoadedImages );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
