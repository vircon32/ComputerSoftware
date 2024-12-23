// *****************************************************************************
    // include Vircon common headers
    #include "../../VirconDefinitions/Constants.hpp"
    
    // include project headers
    #include "RectangleNode.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <stdexcept>    // [ C++ STL ] Exceptions
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      RECTANGLE NODE CLASS
// =============================================================================


RectangleNode::RectangleNode()
{
    MinX = MinY = 0;
    MaxX = MaxY = Constants::GPUTextureSize - 1;
    Part1 = Part2 = nullptr;
    PlacedImage = nullptr;
}

// -----------------------------------------------------------------------------

RectangleNode::RectangleNode( const RectangleNode& Copied )
{
    Part1 = Copied.Part1;
    Part2 = Copied.Part2;
    PlacedImage = Copied.PlacedImage;
}

// -----------------------------------------------------------------------------

RectangleNode::~RectangleNode()
{
    // delete children if partitioned
    if( Part1 )
    {
        delete Part1;
        Part1 = nullptr;
    }
    
    if( Part2 )
    {
        delete Part2;
        Part2 = nullptr;
    }
}

// -----------------------------------------------------------------------------

int RectangleNode::ContentsArea()
{
    // return image area if full
    if( PlacedImage )
      return (MaxX - MinX + 1) * (MaxY - MinY + 1);
    
    // return 0 when empty
    if( !Part1 )
      return 0;
    
    // otherwise area is the sum of partition areas
    return Part1->ContentsArea() + Part2->ContentsArea();
}

// -----------------------------------------------------------------------------

float RectangleNode::UsageProportion()
{
    return (float)ContentsArea() / Area();
}

// -----------------------------------------------------------------------------

void RectangleNode::GetContentsLimit( int& MaxContentsX, int& MaxContentsY )
{
    // recurse to children if partitioned
    if( Part1 )
    {
        int MaxContentsX1, MaxContentsY1;
        int MaxContentsX2, MaxContentsY2;
        Part1->GetContentsLimit( MaxContentsX1, MaxContentsY1 );
        Part2->GetContentsLimit( MaxContentsX2, MaxContentsY2 );
        
        MaxContentsX = max( MaxContentsX1, MaxContentsX2 );
        MaxContentsY = max( MaxContentsY1, MaxContentsY2 );
        return;
    }
    
    // if this rectangle has an image it will match its extents
    if( PlacedImage )
    {
        MaxContentsX = MaxX;
        MaxContentsY = MaxY;
        return;
    }
    
    // if empty provide 2 zeroes
    MaxContentsX = MaxContentsY = 0;
}

// -----------------------------------------------------------------------------

void RectangleNode::DivideInX( int LeftWidth )
{
    Part1 = new RectangleNode;
    Part1->MinY = MinY;
    Part1->MaxY = MaxY;
    Part1->MinX = MinX;
    Part1->MaxX = MinX + LeftWidth - 1;
    
    Part2 = new RectangleNode;
    Part2->MinY = MinY;
    Part2->MaxY = MaxY;
    Part2->MinX = MinX + LeftWidth;
    Part2->MaxX = MaxX;
}

// -----------------------------------------------------------------------------

void RectangleNode::DivideInY( int TopHeight )
{
    Part1 = new RectangleNode;
    Part1->MinX = MinX;
    Part1->MaxX = MaxX;
    Part1->MinY = MinY;
    Part1->MaxY = MinY + TopHeight - 1;
    
    Part2 = new RectangleNode;
    Part2->MinX = MinX;
    Part2->MaxX = MaxX;
    Part2->MinY = MinY + TopHeight;
    Part2->MaxY = MaxY;
}

// -----------------------------------------------------------------------------

bool RectangleNode::CanFitImage( PNGImage& Image )
{
    return (Width() >= Image.Width && Height() >= Image.Height);
}

// -----------------------------------------------------------------------------

void RectangleNode::PlaceImageTopLeft( PNGImage& Image )
{
    if( !CanFitImage( Image ) )
      throw runtime_error( "Rectangle cannot fit image" );
    
    // (1) partition in Y when needed
    if( Height() > Image.Height )
    {
        DivideInY( Image.Height );
        Part1->PlaceImageTopLeft( Image );
    }
    
    // (2) partition in X when needed
    else if( Width() > Image.Width )
    {
        DivideInX( Image.Width );
        Part1->PlaceImageTopLeft( Image );
    }
    
    // (3) set the image
    else PlacedImage = &Image;
}


// =============================================================================
//      ALGORITHMS FOR TREE HANDLING
// =============================================================================


// auxiliary function used to place an image;
// returns the contents area that would result
// or -1 if it can't be placed here
int TryImageIntoRectangle( PNGImage& Image, RectangleNode& Rectangle )
{
    if( !Rectangle.CanFitImage( Image ) )
      return -1;
    
    int MaxContentsX = 0, MaxContentsY = 0;
    Rectangle.GetContentsLimit( MaxContentsX, MaxContentsY );
    
    // expand it with the image at top-left
    MaxContentsX = max( MaxContentsX, Rectangle.MinX + Image.Width - 1 );
    MaxContentsY = max( MaxContentsY, Rectangle.MinY + Image.Height - 1 );
    return (MaxContentsX + 1) * (MaxContentsY + 1);
}

// -----------------------------------------------------------------------------

// returns true if image can be fit here;
// 
bool PlaceImageInTexture_Recursive( PNGImage& Image, RectangleNode& Rectangle, int& MinArea, RectangleNode* MinAreaRect )
{
    bool Fit1 = PlaceImageInTexture_Recursive( Image, *Rectangle.Part1, MinArea, MinAreaRect );
    bool Fit2 = PlaceImageInTexture_Recursive( Image, *Rectangle.Part2, MinArea, MinAreaRect );
    
    // ???
    RectangleNode* CurrentRectangle = &Rectangle;
    
    if( CurrentRectangle->Part1 )
    {
        int Area1 = TryImageIntoRectangle( Image, *CurrentRectangle->Part1 );
        int Area2 = TryImageIntoRectangle( Image, *CurrentRectangle->Part2 );
        
        int MinArea = 0;
        
        if( Area1 > 0 && Area1 < MinArea ) MinArea = Area1;
        if( Area2 > 0 && Area2 < MinArea ) MinArea = Area2;
    }
    
    return Fit1 || Fit2;
}

// -----------------------------------------------------------------------------

bool PlaceImageInTexture( PNGImage& Image )
{
    /* traverse the tree and look for a possible rectangle to insert
       the image; for each possible one determine the limits and do
       the insertion where the limits would yield the least area */
    
    RectangleNode* MinAreaRect = nullptr;
    int MinArea = 0;
    
    PlaceImageInTexture_Recursive( Image, TextureRectangle, MinArea, MinAreaRect );
    
    // check if we didn't find a suitable location
    if( !MinAreaRect )
      return false;
    
    // place the image in the chosen rectangle
    MinAreaRect->PlaceImageTopLeft( Image );
    return true;
}

// -----------------------------------------------------------------------------

void PlaceAllImages()
{
    // sort images, this is important
    LoadedImages.sort();
    
    // now place them all in that order
    for( PNGImage& Image: LoadedImages )
      if( !PlaceImageInTexture( Image ) )
        throw runtime_error( "Cannot fit all images in the texture" );
}
