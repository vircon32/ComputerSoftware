// *****************************************************************************
    // include console logic headers
    #include "ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
      #include <locale>         // [ C++ STL ] Locales
      #include <codecvt>        // [ C++ STL ] Encoding conversions
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace V32
{
    // =============================================================================
    //      CALLBACKS FOR EXTERNAL FUNCTIONS
    // =============================================================================
    
    
    namespace Callbacks
    {
        // callbacks to the video library
        void( *ClearScreen )( V32::GPUColor ) = nullptr;
        void( *DrawQuad )( V32::GPUQuad& ) = nullptr;
        void( *SetMultiplyColor )( V32::GPUColor ) = nullptr;
        void( *SetBlendingMode )( int ) = nullptr;
        void( *SelectTexture )( int ) = nullptr;
        void( *LoadTexture )( int, void* ) = nullptr;
        void( *UnloadCartridgeTextures )() = nullptr;
        void( *UnloadBiosTexture )() = nullptr;
        
        // callbacks to the log library
        void( *LogLine )( const string& ) = nullptr;
        void( *ThrowException )( const string& ) = nullptr;
    }
    
    
    // =============================================================================
    //      WRAPPERS FOR PROPER FILE ACCESS ON UNICODE PATHS
    // =============================================================================
    
    
    void OpenInputFile( ifstream& InputFile, const string& FilePathUTF8, ios_base::openmode Mode )
    {
        #if defined(WINDOWS_OS)
          wstring_convert< codecvt_utf8_utf16< wchar_t > > Converter;
          wstring FilePathUTF16 = Converter.from_bytes( FilePathUTF8 );
          InputFile.open( FilePathUTF16.c_str(), Mode );
        #else
          InputFile.open( FilePathUTF8.c_str(), Mode );
        #endif
    }
    
    // -----------------------------------------------------------------------------

    void OpenOutputFile( ofstream& OutputFile, const string& FilePathUTF8, ios_base::openmode Mode )
    {
        #if defined(WINDOWS_OS)
          wstring_convert< codecvt_utf8_utf16< wchar_t > > Converter;
          wstring FilePathUTF16 = Converter.from_bytes( FilePathUTF8 );
          OutputFile.open( FilePathUTF16.c_str(), Mode );
        #else
          OutputFile.open( FilePathUTF8.c_str(), Mode );
        #endif
    }
    
    // -----------------------------------------------------------------------------

    void OpenInputOutputFile( fstream& IOFile, const string& FilePathUTF8, ios_base::openmode Mode )
    {
        #if defined(WINDOWS_OS)
          wstring_convert< codecvt_utf8_utf16< wchar_t > > Converter;
          wstring FilePathUTF16 = Converter.from_bytes( FilePathUTF8 );
          IOFile.open( FilePathUTF16.c_str(), Mode );
        #else
          IOFile.open( FilePathUTF8.c_str(), Mode );
        #endif
    }
}
