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


bool VerboseMode = false;
int GapBetweenImages = 1;
float HotspotProportionX = 0;
float HotspotProportionY = 0;
list< PNGImage > LoadedImages;
RectangleNode TextureRectangle;


// =============================================================================
//      CODE GENERATION FUNCTIONS
// =============================================================================


void SaveRegionEditorProject( const string& FilePath )
{
    // open output file as text
    ofstream XMLFile;
    XMLFile.open( FilePath );
    
    if( !XMLFile.good() )
      throw runtime_error( "cannot create region editor project file" );
    
    // write XML start
    XMLFile << "<texture-regions>" << endl;
    
    // write texture name
    string TextureName = GetPathFileName( FilePath );
    TextureName = GetFileWithoutExtension( TextureName );
    XMLFile << "    <texture name=\"" << TextureName << "\" path=\"" << TextureName << ".png\" />" << endl;
    
    // write all individual regions
    //     <region name="HeliFuel" left="1" top="1" right="127" bottom="21" hotspot-x="1" hotspot-y="1"/>
    
    // write all region matrices
    //    <matrix columns = "6" rows="1" separation="1">
    //        <region name="Explosion" left="3" top="3" right="74" bottom="74" hotspot-x="38" hotspot-y="38"/>
    //    </matrix>
    
    // write XML end
    XMLFile << "</texture-regions>" << endl;
    XMLFile.close();
}
