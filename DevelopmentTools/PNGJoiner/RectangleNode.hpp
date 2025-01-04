// *****************************************************************************
    // start include guard
    #ifndef RECTANGLENODE_HPP
    #define RECTANGLENODE_HPP
    
    // include project headers
    #include "PNGImage.hpp"
    
    // include C/C++ headers
    #include <vector>       // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      BINARY TREE FOR TEXTURE SUBDIVISION IN RECTANGLES
// =============================================================================


class RectangleNode
{
    public:
        
        // occupied part of the texture rectangle
        int MinX, MaxX;
        int MinY, MaxY;
        
        // placed image, must match rectangle dimensions;
        // nullptr if this rectangle has no image
        PNGImage* PlacedImage;
        
        // subdivisions of this rectangle, always done
        // either cutting on X (order: left, right)
        // or cutting on Y (order: top, down)
        RectangleNode *Part1, *Part2;
        
    public:
        
        // instance handling
        RectangleNode();
        RectangleNode( const RectangleNode& Copied );
       ~RectangleNode();
        
        // properties (in pixels)
        int Width()  { return MaxX - MinX + 1; }
        int Height() { return MaxY - MinY + 1; }
        int Area() { return Width() * Height(); }
        
        // other metrics dependent on its contents
        int ContentsArea();         // in pixels
        float UsageProportion();    // in range [0,1]
        
        // provides the max X and Y coordinates that contain
        // pixels, used to determine actual texture size
        // (returns both as 0 when empty)
        void GetContentsLimit( int& MaxContentsX, int& MaxContentsY );
        
        // subdivision operations
        void DivideInX( int LeftWidth );
        void DivideInY( int TopHeight );
        
        // place an image in top corner (returns -1 if it won't fit??)
        // returns the bounding rectangle's area in pixels after placing
        bool CanFitImage( PNGImage& Image );
        void PlaceImageTopLeft( PNGImage& Image );
};


// =============================================================================
//      DATA FOR TENTATIVE IMAGE PLACEMENTS IN RECTANGLES
// =============================================================================


struct ImagePlacement
{
    public:
    
        RectangleNode* Rectangle;
        float RectangleUsePercentage;
        int TotalTextureArea;
    
    public:
    
        ImagePlacement();
        ImagePlacement( const ImagePlacement& Copied );
};

// sorting image placements
bool operator<( const ImagePlacement& Placement1, const ImagePlacement& Placement2 );


// =============================================================================
//      ALGORITHMS FOR TREE HANDLING
// =============================================================================


bool PlaceImageInTexture( PNGImage& Image );
void PlaceAllImages();
void CreateOutputImage( PNGImage& OutputImage, RectangleNode& Rectangle );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
