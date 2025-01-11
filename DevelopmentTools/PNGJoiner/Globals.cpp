// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>       // [ C++ STL ] I/O Streams
    #include <fstream>        // [ C++ STL ] File Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


// program configuration parameters
bool VerboseMode = false;
int GapBetweenImages = 1;
float HotspotProportionX = 0;
float HotspotProportionY = 0;

// working objects
list< PNGImage > LoadedImages;
list< PNGImage* > SortedImages;
RectangleNode TextureRectangle;


// =============================================================================
//      CODE GENERATION FUNCTIONS
// =============================================================================


// traverses rectangle partitions to find where
// a particular image was placed by the algorithm
RectangleNode* FindImageRectangle( PNGImage* Image, RectangleNode* BaseRectangle )
{
    if( BaseRectangle->Part1 )
    {
        RectangleNode* Result1 = FindImageRectangle( Image, BaseRectangle->Part1 );
        RectangleNode* Result2 = FindImageRectangle( Image, BaseRectangle->Part2 );
        return (Result1? Result1 : Result2);
    }
    
    if( BaseRectangle->PlacedImage == Image )
      return BaseRectangle;
    
    else return nullptr;
}

// -----------------------------------------------------------------------------

void ExportSingleRegion( PNGImage& Image, ofstream& XMLFile )
{
    // find where this image was placed
    RectangleNode* ImagePlacement = FindImageRectangle( &Image, &TextureRectangle );
    
    if( !ImagePlacement )
      throw runtime_error( "Cannot find rectangle where image was placed" );
    
    // determine basic properties
    int MinX = ImagePlacement->MinX;
    int MaxX = MinX + Image.Width - 1;
    int HotspotX = MinX + (Image.Width-1) * HotspotProportionX;
    
    int MinY = ImagePlacement->MinY;
    int MaxY = MinY + Image.Height - 1;
    int HotspotY = MinY + (Image.Height-1) * HotspotProportionY;
    
    // export all properties to XML
    XMLFile << "    " << "<region";
    XMLFile << " name=\"" << Image.Name << "\"";
    XMLFile << " left=\"" << MinX << "\"";
    XMLFile << " top=\"" << MinY << "\"";
    XMLFile << " right=\"" << MaxX << "\"";
    XMLFile << " bottom=\"" << MaxY << "\"";
    XMLFile << " hotspot-x=\"" << HotspotX << "\"";
    XMLFile << " hotspot-y=\"" << HotspotY << "\"";
    XMLFile << "/>" << endl;
}

// -----------------------------------------------------------------------------

void ExportRegionMatrix( PNGImage& Image, ofstream& XMLFile )
{
    // find where this image was placed
    RectangleNode* ImagePlacement = FindImageRectangle( &Image, &TextureRectangle );
    
    if( !ImagePlacement )
      throw runtime_error( "Cannot find rectangle where image was placed" );
    
    // determine single region properties
    int RegionWidth  = (Image.Width  - (Image.TilesX-1) * Image.TilesGap) / Image.TilesX;
    int RegionHeight = (Image.Height - (Image.TilesY-1) * Image.TilesGap) / Image.TilesY;
    
    // determine basic properties
    int MinX = ImagePlacement->MinX;
    int MaxX = MinX + RegionWidth - 1;
    int HotspotX = MinX + (RegionWidth-1) * HotspotProportionX;
    
    int MinY = ImagePlacement->MinY;
    int MaxY = MinY + RegionHeight - 1;
    int HotspotY = MinY + (RegionHeight-1) * HotspotProportionY;
    
    // export matrix properties to XML
    XMLFile << "    " << "<matrix";
    XMLFile << " columns=\"" << Image.TilesX << "\"";
    XMLFile << " rows=\"" << Image.TilesY << "\"";
    XMLFile << " separation=\"" << Image.TilesGap << "\"";
    XMLFile << ">" << endl;
    
    // export first region properties
    XMLFile << "        " << "<region";
    XMLFile << " name=\"" << Image.Name << "\"";
    XMLFile << " left=\"" << MinX << "\"";
    XMLFile << " top=\"" << MinY << "\"";
    XMLFile << " right=\"" << MaxX << "\"";
    XMLFile << " bottom=\"" << MaxY << "\"";
    XMLFile << " hotspot-x=\"" << HotspotX << "\"";
    XMLFile << " hotspot-y=\"" << HotspotY << "\"";
    XMLFile << "/>" << endl;
    
    // close matrix element
    XMLFile << "    " << "</matrix>" << endl;
}

// -----------------------------------------------------------------------------

void SaveRegionEditorProject( const string& FilePath )
{
    // open output file as text
    ofstream XMLFile;
    OpenOutputFile( XMLFile, FilePath );
    
    if( !XMLFile.good() )
      throw runtime_error( "cannot create region editor project file" );
    
    // write XML start
    XMLFile << "<texture-regions>" << endl;
    
    // write texture name
    string TextureName = GetPathFileName( FilePath );
    TextureName = GetFileWithoutExtension( TextureName );
    XMLFile << "    <texture name=\"" << TextureName << "\" path=\"" << TextureName << ".png\" />" << endl;
    
    // export all regions
    for( PNGImage* Image: SortedImages )
    {
        if( Image->TilesX > 1 || Image->TilesY > 1 )
          ExportRegionMatrix( *Image, XMLFile );
        
        else
          ExportSingleRegion( *Image, XMLFile );
    }
    
    // write XML end
    XMLFile << "</texture-regions>" << endl;
    XMLFile.close();
}
