// *****************************************************************************
    // start include guard
    #ifndef ROMDEFINITION_HPP
    #define ROMDEFINITION_HPP
    
    // include common Vircon headers
    #include "../../VirconDefinitions/Constants.hpp"
    #include "../../VirconDefinitions/DataStructures.hpp"
    #include "../../VirconDefinitions/FileFormats.hpp"
    
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
        bool IsBios;
        uint32_t VirconVersion, VirconRevision;
        uint32_t ROMVersion, ROMRevision;
        
        // file paths
        int ExtractedTextures;
        int ExtractedSounds;
        
        // base folder of the definition paths
        std::string RomFileName;
        std::string BaseFolder;
        
    private:
        
        // secondary functions
        void ExtractBinary( V32::BinaryFileFormat::Header& BinaryHeader, std::vector< V32::V32Word >& BinaryWords );
        void ExtractTexture( V32::TextureFileFormat::Header& TextureHeader, std::vector< V32::V32Word >& TexturePixels );
        void ExtractSound( V32::SoundFileFormat::Header& SoundHeader, std::vector< V32::V32Word >& SoundSamples );
        void CreateDefinitionXML();
        void CreateMakeBAT();
        void CreateMakeSH();
        
    public:
        
        // main methods
        void UnpackROM( const std::string& InputPath, const std::string& OutputPath );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
