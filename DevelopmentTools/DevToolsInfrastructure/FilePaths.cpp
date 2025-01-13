// *****************************************************************************
    // include project headers
    #include "FilePaths.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <algorithm>        // [ C++ STL ] Algorithms
    #include <sys/stat.h>       // [ ANSI C ] File status
    
    // these includes are only for unicode conversions
    #if defined(WINDOWS_OS)
      #define WINDOWS_OS
      #include <locale>         // [ C++ STL ] Locales
      #include <codecvt>        // [ C++ STL ] Encoding conversions
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      PATH SEPARATOR CHARACTER
// =============================================================================


// this is dependent on the host operating system
#if defined(WINDOWS_OS)
  const char PathSeparator = '\\';
#else
  const char PathSeparator = '/';
#endif

// -----------------------------------------------------------------------------

// auxiliary function to unify forward and back slashes
string NormalizePathSeparators( const string& FilePath )
{
    string NormalizedPath = FilePath;
    
    for( auto& c: NormalizedPath )
    {
        #if defined(WINDOWS_OS)
          if( c == '/' ) c = '\\';
        #else
          if( c == '\\' ) c = '/';
        #endif
    }
    
    return NormalizedPath;
}


// =============================================================================
//      FILE PATH MANIPULATION FUNCTIONS
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
    string NormalizedPath = NormalizePathSeparators( FilePath );
    size_t SlashPosition = NormalizedPath.rfind( PathSeparator );
    
    // careful, if the path is empty (i.e. current folder)
    // we need to return a dot or else paths will be wrong
    if( SlashPosition == string::npos )
      return string(".") + PathSeparator;
    
    return NormalizedPath.substr( 0, SlashPosition+1 );
}

// -----------------------------------------------------------------------------

string GetPathFileName( const string& FilePath )
{
    string NormalizedPath = NormalizePathSeparators( FilePath );
    size_t SlashPosition = NormalizedPath.rfind( PathSeparator );
    
    if( SlashPosition == string::npos )
      return FilePath;
    
    if( NormalizedPath.size() < (SlashPosition+2) )
      return "";
    
    return NormalizedPath.substr( SlashPosition+1, NormalizedPath.size()-1 );
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
    #if defined(WINDOWS_OS)
    
      struct _stat Info;
      wstring FilePathUTF16 = ToUTF16( FilePath );
      
      if( _wstat( FilePathUTF16.c_str(), &Info ) != 0 )
        return false;
      
      return !(Info.st_mode & _S_IFDIR);
      
    #else
        
      struct stat Info;
      
      if( stat( FilePath.c_str(), &Info ) != 0 )
        return false;
      
      return !(Info.st_mode & S_IFDIR);
      
    #endif
}

// -----------------------------------------------------------------------------

bool DirectoryExists( const string& Path )
{
    #if defined(WINDOWS_OS)
    
      struct _stat Info;
      wstring PathUTF16 = ToUTF16( Path );
      
      if( _wstat( PathUTF16.c_str(), &Info ) != 0 )
        return false;
      
      return (Info.st_mode & _S_IFDIR);
      
    #else
        
      struct stat Info;
      
      if( stat( Path.c_str(), &Info ) != 0 )
        return false;
      
      return (Info.st_mode & S_IFDIR);
      
    #endif
}


// =============================================================================
//      CREATING DIRECTORIES
// =============================================================================


// returns true on success
bool CreateNewDirectory( const string& DirectoryPath )
{
    #if defined(WINDOWS_OS)
      wstring DirectoryPathUTF16 = ToUTF16( DirectoryPath );
      int Status = _wmkdir( DirectoryPathUTF16.c_str() );
    #else
      int Status = mkdir( DirectoryPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    #endif
    
    return (Status >= 0);
}


// =============================================================================
//      UNICODE STRING CONVERSIONS UTF-8 <-> UTF-16
// =============================================================================


#if defined(WINDOWS_OS)

  wstring ToUTF16( const string& TextUTF8 )
  {
      wstring_convert< codecvt_utf8_utf16< wchar_t > > Converter;
      return Converter.from_bytes( TextUTF8 );
  }
  
  // -----------------------------------------------------------------------------
  
  string ToUTF8( const wstring& TextUTF16 )
  {
      wstring_convert< codecvt_utf8_utf16< wchar_t > > Converter;
      return Converter.to_bytes( TextUTF16 );
  }

#endif


// =============================================================================
//      WRAPPERS FOR PROPER FILE ACCESS ON UNICODE PATHS
// =============================================================================


void OpenInputFile( ifstream& InputFile, const string& FilePathUTF8, ios_base::openmode Mode )
{
    #if defined(WINDOWS_OS)
      wstring FilePathUTF16 = ToUTF16( FilePathUTF8 );
      InputFile.open( FilePathUTF16.c_str(), Mode );
    #else
      InputFile.open( FilePathUTF8.c_str(), Mode );
    #endif
}

// -----------------------------------------------------------------------------

void OpenOutputFile( ofstream& OutputFile, const string& FilePathUTF8, ios_base::openmode Mode )
{
    #if defined(WINDOWS_OS)
      wstring FilePathUTF16 = ToUTF16( FilePathUTF8 );
      OutputFile.open( FilePathUTF16.c_str(), Mode );
    #else
      OutputFile.open( FilePathUTF8.c_str(), Mode );
    #endif
}

// -----------------------------------------------------------------------------

FILE* OpenInputFile( const std::string& FilePathUTF8 )
{
    #if defined(WINDOWS_OS)
      wstring FilePathUTF16 = ToUTF16( FilePathUTF8 );
      return _wfopen( FilePathUTF16.c_str(), L"rb" );
    #else
      return fopen( FilePathUTF8.c_str(), "rb" );
    #endif
}

// -----------------------------------------------------------------------------

FILE* OpenOutputFile( const std::string& FilePathUTF8 )
{
    #if defined(WINDOWS_OS)
      wstring FilePathUTF16 = ToUTF16( FilePathUTF8 );
      return _wfopen( FilePathUTF16.c_str(), L"wb" );
    #else
      return fopen( FilePathUTF8.c_str(), "wb" );
    #endif
}
