// *****************************************************************************
    // start include guard
    #ifndef FILEPATHS_HPP
    #define FILEPATHS_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <iostream>         // [ C++ STL ] I/O streams
    #include <fstream>          // [ C++ STL ] File streams
    #include <stdio.h>          // [ ANSI C ] Standard I/O
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
bool CreateDirectory( const std::string DirectoryPath );


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
