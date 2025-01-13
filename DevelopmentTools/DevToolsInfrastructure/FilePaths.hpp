// *****************************************************************************
    // start include guard
    #ifndef FILEPATHS_HPP
    #define FILEPATHS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <iostream>         // [ C++ STL ] I/O streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <stdio.h>          // [ ANSI C ] Standard I/O
    
    // detection of Windows
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
    #endif
// *****************************************************************************


// =============================================================================
//      FILE PATH MANIPULATION FUNCTIONS
// =============================================================================


// path separactor character
extern const char PathSeparator;

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

// creating directories
bool CreateNewDirectory( const std::string& DirectoryPath );


// =============================================================================
//      UNICODE STRING CONVERSIONS UTF-8 <-> UTF-16
// =============================================================================


#if defined(WINDOWS_OS)

  // auxiliary functions for all UTF-8 to UTF-16 string conversions
  std::wstring ToUTF16( const std::string& TextUTF8 );
  std::string ToUTF8( const std::wstring& TextUTF16 );

#endif


// =============================================================================
//      WRAPPERS FOR PROPER FILE ACCESS ON UNICODE PATHS
// =============================================================================


// functions for C++ I/O file streams
void OpenInputFile
(
    std::ifstream& InputFile, const std::string& FilePathUTF8,
    std::ios_base::openmode Mode = std::ios_base::in
);

void OpenOutputFile
(
    std::ofstream& OutputFile, const std::string& FilePathUTF8,
    std::ios_base::openmode Mode = std::ios_base::out
);

// -----------------------------------------------------------------------------

// functions for C FILE pointers
FILE* OpenInputFile ( const std::string& FilePathUTF8 );
FILE* OpenOutputFile( const std::string& FilePathUTF8 );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
