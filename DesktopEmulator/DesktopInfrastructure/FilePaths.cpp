// *****************************************************************************
    // include project headers
    #include "FilePaths.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <algorithm>        // [ C++ STL ] Algorithms
    #include <sys/stat.h>       // [ ANSI C ] File status
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PATH SEPARATOR CHARACTER
// =============================================================================


// this is dependent on the host operating system
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
    char PathSeparator = '\\';
#else
    char PathSeparator = '/';
#endif


// =============================================================================
//      STRING MANIPULATION FUNCTIONS
// =============================================================================


string GetFileExtension( const string& FilePath )
{
    size_t DotPosition = FilePath.rfind( '.' );
    
    if( DotPosition == string::npos )
      return "";
    
    return FilePath.substr( DotPosition+1 );
}

// -----------------------------------------------------------------------------

// returns extension without the dot
string GetFileWithoutExtension( const string& FilePath )
{
    size_t DotPosition = FilePath.rfind( '.' );
    
    if( DotPosition == string::npos )
      return "";
    
    return FilePath.substr( 0, DotPosition );
}

// -----------------------------------------------------------------------------

string ReplaceFileExtension( const string& FilePath, const string& NewExtension )
{
    string OriginalExtension = GetFileExtension( FilePath );
    string PathWithoutExtension = FilePath.substr( 0, FilePath.length() - OriginalExtension.length() );
    return PathWithoutExtension + NewExtension;
}

// -----------------------------------------------------------------------------

string GetPathDirectory( const string& FilePath )
{
    size_t SlashPosition = FilePath.rfind( PathSeparator );
    
    // careful, if the path is empty (i.e. current folder)
    // we need to return a dot or else paths will be wrong
    if( SlashPosition == string::npos )
      return string(".") + PathSeparator;
    
    return FilePath.substr( 0, SlashPosition+1 );
}

// -----------------------------------------------------------------------------

string GetPathFileName( const string& FilePath )
{
    size_t SlashPosition = FilePath.rfind( PathSeparator );
    
    if( SlashPosition == string::npos )
      return "";
    
    if( FilePath.size() < (SlashPosition+2) )
      return "";
    
    return FilePath.substr( SlashPosition+1, FilePath.size()-1 );
}


// =============================================================================
//      CHECKING VALIDITY OF PATHS
// =============================================================================


bool IsFileNameValid( const string& FileName )
{
    if( FileName.find('\"') != string::npos )  return false;
    if( FileName.find('\\') != string::npos )  return false;
    if( FileName.find('/')  != string::npos )  return false;
    if( FileName.find('|')  != string::npos )  return false;
    if( FileName.find('<')  != string::npos )  return false;
    if( FileName.find('>')  != string::npos )  return false;
    if( FileName.find(':')  != string::npos )  return false;
    if( FileName.find('?')  != string::npos )  return false;
    if( FileName.find('*')  != string::npos )  return false;
    
    return true;
}

// -----------------------------------------------------------------------------

bool FileExists( const string& FilePath )
{
   ifstream TestedFile;
   TestedFile.open( FilePath );
   
   return (bool)TestedFile;
}

// -----------------------------------------------------------------------------

bool DirectoryExists( const string& Path )
{
    struct stat Info;

    if( stat( Path.c_str(), &Info ) != 0 )
      return false;
    
    return (Info.st_mode & S_IFDIR);
}
