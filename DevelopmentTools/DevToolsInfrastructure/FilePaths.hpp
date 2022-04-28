// *****************************************************************************
    // start include guard
    #ifndef FILEPATHS_HPP
    #define FILEPATHS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
// *****************************************************************************


// =============================================================================
//      STRING MANIPULATION FUNCTIONS
// =============================================================================


// path separactor character
extern char PathSeparator;

// extension detection and replacement
std::string GetFileExtension( const std::string& FilePath );
std::string GetFileWithoutExtension( const std::string& FilePath );
std::string ReplaceFileExtension( const std::string& FilePath, const std::string& NewExtension );

// directory detection
std::string GetPathDirectory( const std::string& FilePath );
std::string GetPathFileName( const std::string& FilePath );

// checking validity of paths
bool IsFileNameValid( const std::string& FileName );
bool FileExists( const std::string &FilePath );
bool DirectoryExists( const std::string &Path );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
