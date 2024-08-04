// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <sstream>          // [ C++ STL ] String streams
    
    // include TinyXML2 headers
    #include <tinyxml2.h>       // [ TinyXML2 ] Main header
    
    // declare used namespaces
    using namespace std;
    using namespace tinyxml2;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES
// =============================================================================


bool Debug = false;
bool VerboseMode = false;


// =============================================================================
//      XML HELPER FUNCTIONS
// =============================================================================


// automation for child elements in XML
XMLElement* GetRequiredElement( XMLElement* Parent, const string& ChildName )
{
    if( !Parent )
      throw runtime_error( "Parent element NULL" );
    
    XMLElement* Child = Parent->FirstChildElement( ChildName.c_str() );
    
    if( !Child )
      throw runtime_error( string("Cannot find element <") + ChildName + "> inside <" + Parent->Name() + ">" );
    
    return Child;
}

// -----------------------------------------------------------------------------

// automation for string attributes in XML
string GetRequiredStringAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      throw runtime_error( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      throw runtime_error( string("Cannot find attribute '") + AtributeName + "' inside <" + Element->Name() + ">" );
    
    return Attribute->Value();
}

// -----------------------------------------------------------------------------

// automation for integer attributes in XML
int GetRequiredIntegerAttribute( XMLElement* Element, const string& AtributeName )
{
    if( !Element )
      throw runtime_error( "Parent element NULL" );
    
    const XMLAttribute* Attribute = Element->FindAttribute( AtributeName.c_str() );

    if( !Attribute )
      throw runtime_error( string("Cannot find attribute '") + AtributeName + "' inside <" + Element->Name() + ">" );
    
    // attempt integer conversion
    int Number = 0;
    XMLError ErrorCode = Element->QueryIntAttribute( AtributeName.c_str(), &Number );
    
    if( ErrorCode != XML_SUCCESS )
      throw runtime_error( string("Attribute '") + AtributeName + "' inside <" + Element->Name() + "> must be an integer number" );
    
    return Number;
}


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


void PrintUsage()
{
    cout << "USAGE: tiled2vircon [options] file" << endl;
    cout << "Options:" << endl;
    cout << "  --help       Displays this information" << endl;
    cout << "  --version    Displays program version" << endl;
    cout << "  -o <folder>  Output folder, default is the current folder" << endl;
    cout << "  -v           Displays additional information (verbose)" << endl;
    cout << "For each tilemap layer, output file name is the layer name in Tiled" << endl;
}

// -----------------------------------------------------------------------------

void PrintVersion()
{
    cout << "tiled2vircon v24.8.4" << endl;
    cout << "Vircon32 Tiled tilemap importer by Javier Carracedo" << endl;
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char** Arguments )
{
    try
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process command line arguments
        
        // variables to capture input parameters
        string InputPath, OutputFolder;
        
        // process arguments
        for( int i = 1; i < NumberOfArguments; i++ )
        {
            if( Arguments[i] == string("--help") )
            {
                PrintUsage();
                return 0;
            }
            
            if( Arguments[i] == string("--version") )
            {
                PrintVersion();
                return 0;
            }
            
            if( Arguments[i] == string("-v") )
            {
                VerboseMode = true;
                continue;
            }
            
            if( Arguments[i] == string("-o") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing folder name after '-o'" );
                
                // now we can safely read the input path
                OutputFolder = Arguments[ i ];
                continue;
            }
            
            // discard any other parameters starting with '-'
            if( Arguments[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + Arguments[i] + "'" );
            
            // any non-option parameter is taken as the input file
            if( InputPath.empty() )
            {
                InputPath = Arguments[i];
            }
            
            // only a single input file is supported!
            else
              throw runtime_error( "too many input files, only 1 is supported" );
        }
        
        // check if an input path was given
        if( InputPath.empty() )
          throw runtime_error( "no input file" );
        
        // if output path was not given, just
        // replace the extension in the input
        if( OutputFolder.empty() )
        {
            OutputFolder = string(".") + PathSeparator;
            
            if( VerboseMode )
              cout << "using output folder: \"" << OutputFolder << "\"" << endl;
        }
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 1: Open the source XML and read basic information
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // try to open the source file as an XML document
        XMLDocument LoadedMap;
        XMLError ErrorCode = LoadedMap.LoadFile( InputPath.c_str() );
        
        if( ErrorCode != XML_SUCCESS )
          throw runtime_error( "Cannot read XML from file path " + InputPath );
          
        // obtain XML root
        XMLElement* MapRoot = LoadedMap.FirstChildElement( "map" );
        
        if( !MapRoot )
          throw runtime_error( "Cannot find <map> root element" );
        
        // obtain map dimensions
        int MapWidth = GetRequiredIntegerAttribute( MapRoot, "width" );
        int MapHeight = GetRequiredIntegerAttribute( MapRoot, "height" );
        
        // in Tiled, tile IDs never start from 0, so
        // identify the first tile ID for the tileset
        XMLElement* MapTileSet = GetRequiredElement( MapRoot, "tileset" );
        int FirstTileID = GetRequiredIntegerAttribute( MapTileSet, "firstgid" );
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // STEP 2: Save each tile map layers to a separate file
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // iterate all layers, saving each layer to
        // a different embedded file using the layer name
        XMLElement* MapLayer = MapRoot->FirstChildElement( "layer" );
        
        while( MapLayer != nullptr )
        {
            // read its name
            string LayerName = GetRequiredStringAttribute( MapLayer, "name" );
            string FilePath = OutputFolder + LayerName + ".vmap";
            
            cout << "Saving layer \"" << FilePath << "\"" << endl;
            
            // create a file with that name
            ofstream OutputFile;
            OutputFile.open( FilePath, ios_base::binary | ios_base::trunc );
            
            if( !OutputFile.good() )
              throw runtime_error( "Cannot open output file" );
              
            // now transfer the actual tile data
            XMLElement* TileData = GetRequiredElement( MapLayer, "data" );
            string TilesText = TileData->GetText();
            
            // load all tile positions as a CSV
            // using a stringstream to make it easier
            stringstream TilesStream( TilesText );
            int NumberOfTiles = 0;
            
            while( TilesStream )
            {
                // get next number using comma as delimiter
                string ValueText;
                if( !getline( TilesStream, ValueText, ',' ) ) break;
                
                // convert number string to integer
                uint32_t Number = atoi( ValueText.c_str() );
                
                // save the tile value normalized to the first id
                if( (int)Number < FirstTileID )
                  Number = FirstTileID;
                
                Number -= FirstTileID;
                OutputFile.write( (char*)(&Number), 4 );
                
                // count the tiles
                NumberOfTiles++;
            }
            
            // check that we have obtained the correct number of tiles
            if( NumberOfTiles != (MapWidth * MapHeight) )
              cout << "Warning: Expected " << MapWidth << "x" << MapHeight << " tiles (total "
                   << MapWidth * MapHeight << "), found " << NumberOfTiles << endl;
            
            // go to next layer
            OutputFile.close();
            MapLayer = MapLayer->NextSiblingElement( "layer" );
        }
    }
    
    catch( const exception& e )
    {
        cerr << "tiled2vircon: error: " << e.what() << endl;
        return 1;
    }
    
    // report success
    if( VerboseMode )
      cout << "conversion successful" << endl;
    
    return 0;
}
