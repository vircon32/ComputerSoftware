// *****************************************************************************
    // start include guard
    #ifndef WAVFORMAT_HPP
    #define WAVFORMAT_HPP
// *****************************************************************************


// =============================================================================
//      DATA STRUCTURES IN A WAV FILE
// =============================================================================


typedef struct
{
    char     ChunkID[ 4 ];      // signature: the 4 characters "RIFF"
    uint32_t ChunkSize;         // size in bytes, not including chunkSize or chunkID
    char     Format[ 4 ];       // for a WAV file, the 4 characters "WAVE"
}
RIFFChunkHeader;

// -----------------------------------------------------------------------------

typedef struct
{
    char     SubchunkID[ 4 ];   // signature: the 4 characters "RIFF"
    uint32_t SubchunkSize;      // size in bytes, not including this header
}
SubchunkHeader;

// -----------------------------------------------------------------------------

// subchunk signature: the 4 characters "fmt "
typedef struct
{
    uint16_t AudioFormat;       // we only support PCM = 1, other formats are compressed
    uint16_t NumberOfChannels;  // we only support 2 = Stereo
    uint32_t SampleRate;        // we only support 44100 Hz
    uint32_t ByteRate;          // ByteRate = SampleRate * Channels * BitsPerSample/8
    uint16_t BlockAlign;        // BlockAlign = Channels * BitsPerSample/8
    uint16_t BitsPerSample;     // we only support 16 bits per sample
}
FormatSubchunkBody;

// -----------------------------------------------------------------------------

// (the data subchunk only has the subchunk header and
//  the sample data, so no data subchunk body exists)
// subchunk signature: the 4 characters "data"

// -----------------------------------------------------------------------------

typedef struct
{
    int16_t LeftSample;
    int16_t RightSample;
}
SoundSample;


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
