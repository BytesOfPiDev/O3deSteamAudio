#pragma once

#include "Engine/AudioInput/AudioInputFile.h"

namespace SteamAudio
{
    /**
     * A RIFF format chunk header.
     */
    struct ChunkHeader
    {
        AZ::u8 tag[4];
        AZ::u32 size;
    };

    /**
     * A WAVE format "fmt" chunk.
     */
    struct FmtChunk
    {
        ChunkHeader header;
        AZ::u16 audioFormat;
        AZ::u16 numChannels;
        AZ::u32 sampleRate;
        AZ::u32 byteRate;
        AZ::u16 blockAlign;
        AZ::u16 bitsPerSample;
    };

    /**
     * A WAVE format header.
     */
    struct WavHeader
    {
        ChunkHeader riff;
        AZ::u8 wave[4];
        FmtChunk fmt;
        ChunkHeader data;

        static const size_t MinSize = 44;
    };

    static_assert(sizeof(WavHeader) == WavHeader::MinSize, "WavHeader struct size is not 44 bytes!");


    /**
     * Type of AudioFileParser for Wav File Format.
     * Parses header information from Wav files and stores it for retrieval.
     */
    class WavFileParser
        : public AudioFileParser
    {
    public:
        AUDIO_IMPL_CLASS_ALLOCATOR(WavFileParser)

        WavFileParser();
        ~WavFileParser() override;

        auto ParseHeader(AZ::IO::FileIOStream& fileStream) -> size_t override;

        [[nodiscard]] auto IsHeaderValid() const -> bool override;

        [[nodiscard]] auto GetSampleType() const -> Audio::AudioInputSampleType override;
        [[nodiscard]] auto GetNumChannels() const -> AZ::u32 override;
        [[nodiscard]] auto GetSampleRate() const -> AZ::u32 override;
        [[nodiscard]] auto GetByteRate() const -> AZ::u32 override;
        [[nodiscard]] auto GetBitsPerSample() const -> AZ::u32 override;
        [[nodiscard]] auto GetDataSize() const -> AZ::u32 override;

    private:
        static auto ValidTag(const AZ::u8 tag[4], const AZ::u8 name[4]) -> bool;

        WavHeader m_header;
        bool m_headerIsValid = false;

        static const AZ::u8 riff_tag[4];
        static const AZ::u8 wave_tag[4];
        static const AZ::u8 fmt__tag[4];
        static const AZ::u8 data_tag[4];
    };


    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::IsHeaderValid() const -> bool
    {
        return m_headerIsValid;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::GetNumChannels() const -> AZ::u32
    {
        return m_header.fmt.numChannels;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::GetSampleRate() const -> AZ::u32
    {
        return m_header.fmt.sampleRate;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::GetByteRate() const -> AZ::u32
    {
        return m_header.fmt.byteRate;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::GetBitsPerSample() const -> AZ::u32
    {
        return m_header.fmt.bitsPerSample;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    inline auto WavFileParser::GetDataSize() const -> AZ::u32
    {
        return m_header.data.size;
    }

} // namespace Audio
