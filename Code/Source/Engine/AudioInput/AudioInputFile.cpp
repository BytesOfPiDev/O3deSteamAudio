#include "Engine/AudioInput/AudioInputFile.h"

#include "AzCore/IO/FileIO.h"

#include "Engine/AudioInput/WavParser.h"

namespace SteamAudio
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Audio Input File
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    AudioInputFile::AudioInputFile(Audio::SAudioInputConfig const& sourceConfig)
    {
        m_config = sourceConfig;

        switch (sourceConfig.m_sourceType)
        {
        case Audio::AudioInputSourceType::WavFile:
            m_parser.reset(aznew WavFileParser());
            break;
        case Audio::AudioInputSourceType::PcmFile:
            break;
        default:
            return;
        }

        LoadFile();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    AudioInputFile::~AudioInputFile()
    {
        UnloadFile();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioInputFile::LoadFile() -> bool
    {
        bool result = false;

        // Filename should be relative to the project assets root e.g.: 'sounds/files/my_sound.wav'
        AZ::IO::FileIOStream fileStream(
            m_config.m_sourceFilename.c_str(),
            AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary);

        if (fileStream.IsOpen())
        {
            m_dataSize = fileStream.GetLength();

            if (m_dataSize > 0)
            {
                // Here if a parser is available, can pass the file stream forward
                // so it can parse header information.
                // It will return the number of header bytes read, that is an offset to
                // the beginning of the real signal data.
                if (m_parser)
                {
                    size_t headerBytesRead = m_parser->ParseHeader(fileStream);
                    if (headerBytesRead > 0 && m_parser->IsHeaderValid())
                    {
                        // Update the size...
                        m_dataSize = m_parser->GetDataSize();

                        // Set the format configuration obtained from the file...
                        m_config.m_bitsPerSample = m_parser->GetBitsPerSample();
                        m_config.m_numChannels = m_parser->GetNumChannels();
                        m_config.m_sampleRate = m_parser->GetSampleRate();
                        m_config.m_sampleType = m_parser->GetSampleType();
                    }
                }

                if (IsOk())
                {
                    // Allocate a new buffer to hold the data...
                    m_dataPtr = new AZ::u8[m_dataSize];

                    // Read file into internal buffer...
                    size_t bytesRead = fileStream.Read(m_dataSize, m_dataPtr);

                    ResetBookmarks();

                    // Verify we read the full amount...
                    result = (bytesRead == m_dataSize);
                }
            }

            fileStream.Close();
        }

        if (result)
        {
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioInputFile::UnloadFile()
    {
        if (m_dataPtr)
        {
            delete[] m_dataPtr;
            m_dataPtr = nullptr;
        }
        m_dataSize = 0;
        m_dataCurrentPtr = nullptr;
        m_dataCurrentReadSize = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioInputFile::ReadInput([[maybe_unused]] Audio::AudioStreamData const& data)
    {
        // Don't really need this for File-based sources, the whole file is read in the constructor.
        // However, we may need to implement this for asynchronous loading of the file (streaming).
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioInputFile::WriteOutput([[maybe_unused]] IPLAudioBuffer* akBuffer)
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioInputFile::IsOk() const -> bool
    {
        bool ok = (m_dataSize > 0);
        ok &= IsFormatValid();

        if (m_parser)
        {
            ok &= m_parser->IsHeaderValid();
            ok &= (m_dataSize == m_parser->GetDataSize());
        }

        return ok;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioInputFile::OnDeactivated()
    {
        if (m_config.m_autoUnloadFile)
        {
            UnloadFile();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioInputFile::CopyData(size_t numSampleFrames, void* toBuffer) -> size_t
    {
        // Copies data to an output buffer.
        // Size requested is in sample frames, not bytes!
        // Number of frames actually copied is returned.  This is useful if more
        // frames were requested than can be copied.

        if (!toBuffer || !numSampleFrames)
        {
            return 0;
        }

        size_t const frameBytes =
            (m_config.m_numChannels * m_config.m_bitsPerSample) >> 3; // bits --> bytes
        size_t copySize = numSampleFrames * frameBytes;

        // Check if request is larger than remaining, trim off excess.
        if (m_dataCurrentReadSize + copySize > m_dataSize)
        {
            size_t excess = (m_dataCurrentReadSize + copySize) - m_dataSize;
            copySize -= excess;
            numSampleFrames = (copySize / frameBytes);
        }

        if (copySize > 0)
        {
            ::memcpy(toBuffer, m_dataCurrentPtr, copySize);
            m_dataCurrentReadSize += copySize;
            m_dataCurrentPtr += copySize;
        }

        return numSampleFrames;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioInputFile::ResetBookmarks()
    {
        m_dataCurrentPtr = m_dataPtr;
        m_dataCurrentReadSize = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioInputFile::IsEof() const -> bool
    {
        return (m_dataCurrentReadSize == m_dataSize);
    }

} // namespace SteamAudio
