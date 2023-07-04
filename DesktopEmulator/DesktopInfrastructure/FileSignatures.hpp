// *****************************************************************************
    // start include guard
    #ifndef FILESIGNATURES_HPP
    #define FILESIGNATURES_HPP
    
    // include C/C++ headers
    #include <cstdint>          // [ ANSI C ] Standard integer types
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <fstream>          // [ C++ STL ] File streams
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO WORK WITH SIGNATURES
// =============================================================================


void WriteSignature( std::ofstream& OutputFile, const char* Value );
bool CheckSignature( char* Signature, const char* Expected );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

