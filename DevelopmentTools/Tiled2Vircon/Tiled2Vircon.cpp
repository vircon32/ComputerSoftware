// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <sstream>          // [ C++ STL ] String streams
    #include <vector>           // [ C++ STL ] Vectors
    
    // include TinyXML2 headers
    #include <tinyxml2.h>       // [ TinyXML2 ] Main header
    
    // on Windows include headers for unicode conversion
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
      #include <windows.h>      // [ WINDOWS ] Main header
      #include <shellapi.h>     // [ WINDOWS ] Shell API
    #endif
    
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
    cout << "tiled2vircon v25.1.4" << endl;
    cout << "Vircon32 Tiled tilemap importer by Javier Carracedo" << endl;
}


// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char* Arguments[] )
{
    try
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process command line arguments
        
        // variables to capture input parameters
        string InputPath, OutputFolder;
        
        // to treat arguments the same in any OS we
        // will convert them to UTF-8 in all cases
        vector< string > ArgumentsUTF8;
        
        #if defined(WINDOWS_OS)
        
          // on Windows we can't rely on the arguments received
          // in main: ask Windows for the UTF-16 command line
          wchar_t* CommandLineUTF16 = GetCommandLineW();
          wchar_t** ArgumentsUTF16 = CommandLineToArgvW( CommandLineUTF16, &NumberOfArguments );
          
          // now convert every program argument to UTF-8
          for( int i = 0; i < NumberOfArguments; i++ )
            ArgumentsUTF8.push_back( ToUTF8( ArgumentsUTF16[i] ) );
          
          LocalFree( ArgumentsUTF16 );
          
        #else
            
          // on Linux/Mac arguments in main are already UTF-8
          for( int i = 0; i < NumberOfArguments; i++ )
            ArgumentsUTF8.push_back( Arguments[i] );
        
        #endif
        
        // process arguments
        for( int i = 1; i < NumberOfArguments; i++ )
        {
            if( ArgumentsUTF8[i] == string("--help") )
            {
                PrintUsage();
                return 0;
            }
            
            if( ArgumentsUTF8[i] == string("--version") )
            {
                PrintVersion();
                return 0;
            }
            
            if( ArgumentsUTF8[i] == string("-v") )
            {
                VerboseMode = true;
                continue;
            }
            
            if( ArgumentsUTF8[i] == string("-o") )
            {
                // expect another argument
                i++;
                
                if( i >= NumberOfArguments )
                  throw runtime_error( "missing folder name after '-o'" );
                
                // now we can safely read the input path
                OutputFolder = ArgumentsUTF8[ i ];
                continue;
            }
            
            // discard any other parameters starting with '-'
            if( ArgumentsUTF8[i][0] == '-' )
              throw runtime_error( string("unrecognized command line option '") + ArgumentsUTF8[i] + "'" );
            
            // any non-option parameter is taken as the input file
            if( InputPath.empty() )
            {
                InputPath = ArgumentsUTF8[i];
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
        
        FILE* InputFile = nullptr;
        tinyxml2::XMLDocument LoadedMap;
        
        try
        {
            // open the file
            InputFile = OpenInputFile( InputPath );
            
            if( !InputFile )
              throw runtime_error( string("cannot open input file \"") + InputPath + "\"" );
            
            // load file as an XML document
            XMLError ErrorCode = LoadedMap.LoadFile( InputFile );
            
            if( ErrorCode != XML_SUCCESS )
              throw runtime_error( "Cannot read XML from file path " + InputPath );
        
            // close the file
            fclose( InputFile );
            InputFile = nullptr;
        }
        catch(...)
        {
            // ensure the file is never left open
            if( InputFile )
              fclose( InputFile );
            
            // and then rethrow the exception&
            throw;
        }
        
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
            OpenOutputFile( OutputFile, FilePath, ios_base::binary | ios_base::trunc );
            
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
