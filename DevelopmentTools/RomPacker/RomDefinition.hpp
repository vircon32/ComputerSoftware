// *****************************************************************************
    // start include guard
    #ifndef ROMDEFINITION_HPP
    #define ROMDEFINITION_HPP
    
    // include C/C++ headers
    #include <string>               // [ C++ STL ] Strings
    #include <vector>               // [ C++ STL ] Vectors
// *****************************************************************************


// =============================================================================
//      DEFINITION OF ROM CONTENTS
// =============================================================================


class RomDefinition
{
    public:
        
        // rom information
        std::string Title;
        uint32_t Version;
        uint32_t Revision;
        bool IsBios;
        
        // file paths
        std::vector< std::string > TexturePaths;
        std::vector< std::string > SoundPaths;
        std::string BinaryPath;
        
        // base folder of the definition paths
        std::string BaseFolder;
        
    private:
        
        // secondary functions
        void ProcessBinary ( std::string& BinaryPath , std::vector< uint32_t >& ProgramROM );
        void ProcessTexture( std::string& TexturePath, std::vector< uint32_t >& TextureROM );
        void ProcessSound  ( std::string& SoundPath  , std::vector< uint32_t >& SoundROM   );
        
    public:
        
        // main methods
        void LoadXML( const std::string& InputPath );
        void PackROM( const std::string& OutputPath );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
