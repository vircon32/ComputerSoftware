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
    MinX = Copied.MinX;
    MinY = Copied.MinY;
    MaxX = Copied.MaxX;
    MaxY = Copied.MaxY;
    
    // make deep copies of subdivisions; otherwise when one
    // of the parent copies is deleted all children will be too
    Part1 = Part2 = nullptr;
    
    if( Copied.Part1 )
      Part1 = new RectangleNode( *Copied.Part1 );
    
    if( Copied.Part2 )
      Part2 = new RectangleNode( *Copied.Part2 );
    
    // however the image is just a pointer
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
    
    // if empty provide 2x -1 to get area zero
    MaxContentsX = MaxContentsY = -1;
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
    if( PlacedImage ) return false;
    return (Width() >= Image.PaddedWidth() && Height() >= Image.PaddedHeight());
}

// -----------------------------------------------------------------------------

void RectangleNode::PlaceImageTopLeft( PNGImage& Image )
{
    if( !CanFitImage( Image ) )
      throw runtime_error( "Rectangle cannot fit image" );
    
    // (1) partition in X when needed
    if( Width() > Image.PaddedWidth() )
    {
        DivideInX( Image.PaddedWidth() );
        Part1->PlaceImageTopLeft( Image );
    }
    
    // (2) partition in Y when needed
    else if( Height() > Image.PaddedHeight() )
    {
        DivideInY( Image.PaddedHeight() );
        Part1->PlaceImageTopLeft( Image );
    }
    
    // (3) set the image
    else PlacedImage = &Image;
}


// =============================================================================
//      DATA FOR TENTATIVE IMAGE PLACEMENTS IN RECTANGLES
// =============================================================================


ImagePlacement::ImagePlacement()
{
    Rectangle = nullptr;
    RectangleUsePercentage = 0;
    TotalTextureArea = 0;
}

ImagePlacement::ImagePlacement( const ImagePlacement& Copied )
{
    Rectangle = Copied.Rectangle;
    RectangleUsePercentage = Copied.RectangleUsePercentage;
    TotalTextureArea = Copied.TotalTextureArea;
}

bool operator<( const ImagePlacement& Placement1, const ImagePlacement& Placement2 )
{
    // discard wrong cases
    if( !Placement1.Rectangle ) return false;
    if( !Placement2.Rectangle ) return true;
    
    // rule 1: prefer less texture area
    if( Placement1.TotalTextureArea != Placement2.TotalTextureArea )
      return (Placement1.TotalTextureArea < Placement2.TotalTextureArea);
    
    // rule 2: prefer better filled rectangles
    if( Placement1.RectangleUsePercentage != Placement2.RectangleUsePercentage )
      return (Placement1.RectangleUsePercentage < Placement2.RectangleUsePercentage);
    
    // rule 3: prefer smaller rectangles
    return (Placement1.Rectangle->Area() < Placement2.Rectangle->Area());
}

// =============================================================================
//      ALGORITHMS FOR TREE HANDLING
// =============================================================================


void GetPlacementData( PNGImage& Image, RectangleNode& Rectangle, list< ImagePlacement >& PossiblePlacements )
{
    if( !Rectangle.CanFitImage( Image ) )
      return;
    
    // can't place image in a subdivided rectangle
    // (should be done in its children instead)
    if( Rectangle.Part1 )
      return;
    
    // get current texture contents
    int GlobalMaxX, GlobalMaxY;
    TextureRectangle.GetContentsLimit( GlobalMaxX, GlobalMaxY );
    
    // suppose we place the image here at top-left
    int NewMaxX = max( GlobalMaxX, Rectangle.MinX + Image.PaddedWidth() - 1 );
    int NewMaxY = max( GlobalMaxY, Rectangle.MinY + Image.PaddedHeight() - 1 );
    
    // add placement info to the list
    PossiblePlacements.emplace_back();
    PossiblePlacements.back().Rectangle = &Rectangle;
    PossiblePlacements.back().RectangleUsePercentage = 100.0 * Image.PaddedArea() / Rectangle.Area();
    PossiblePlacements.back().TotalTextureArea = (NewMaxX + 1) * (NewMaxY + 1);
}

// -----------------------------------------------------------------------------

void PlaceImageInTexture_Recursive( PNGImage& Image, RectangleNode& Rectangle, list< ImagePlacement >& PossiblePlacements )
{
    // can't directly place images in subdivided rectangles
    if( Rectangle.Part1 )
    {
        PlaceImageInTexture_Recursive( Image, *Rectangle.Part1, PossiblePlacements );
        PlaceImageInTexture_Recursive( Image, *Rectangle.Part2, PossiblePlacements );
        return;
    }
    
    if( !Rectangle.CanFitImage( Image ) )
      return;
    
    // create placement info and add it to the list
    GetPlacementData( Image, Rectangle, PossiblePlacements );
}

// -----------------------------------------------------------------------------

// returns true if image can be fit in the texture
bool PlaceImageInTexture( PNGImage& Image )
{
    // traverse the tree and look for all possible rectangles
    // to insert the image; for each one determine insertion
    // info so that later we can choose the best possible one
    list< ImagePlacement > PossiblePlacements;
    PlaceImageInTexture_Recursive( Image, TextureRectangle, PossiblePlacements );
    
    // check if we didn't find a suitable location
    if( PossiblePlacements.empty() )
      return false;
    
    // place the image in the best placement
    PossiblePlacements.sort();
    PossiblePlacements.front().Rectangle->PlaceImageTopLeft( Image );
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

// -----------------------------------------------------------------------------

// this will traverse the rectangle partitions, copying all
// placed images onto the final image at the right coordinates
void CreateOutputImage( PNGImage& OutputImage, RectangleNode& Rectangle )
{
    if( Rectangle.Part1 )
      CreateOutputImage( OutputImage, *Rectangle.Part1 );
    
    if( Rectangle.Part2 )
      CreateOutputImage( OutputImage, *Rectangle.Part2 );
    
    if( !Rectangle.PlacedImage )
      return;
    
    OutputImage.CopySubImage( *Rectangle.PlacedImage, Rectangle.MinX, Rectangle.MinY );
}
