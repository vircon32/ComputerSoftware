// *****************************************************************************
    // start include guard
    #ifndef PNGIMAGE_HPP
    #define PNGIMAGE_HPP
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
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
        void PlaceSubImage( const PNGImage& SubImage, int LeftX, int TopY );
};


// =============================================================================
//      SORTING IMAGES
// =============================================================================


bool operator<( const PNGImage& Image1, const PNGImage& Image2 );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
