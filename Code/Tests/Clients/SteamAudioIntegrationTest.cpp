#include <AzTest/AzTest.h>

#include <cstddef>

#include "AzCore/IO/FileIO.h"
#include "AzCore/std/containers/vector.h"
#include "AzCore/std/string/string.h"

#include "phonon.h"

#include "Clients/BaseTestFixture.h"
#include "Engine/AudioSource.h"

static constexpr auto TestInputFile{ "@gemroot:SteamAudio@/Test/Assets/inputaudio.raw" };
static constexpr auto TestSamplingRate = 44100;
static constexpr auto TestFrameSize = 1024;
static constexpr auto TestBitsPerSample = 32;

auto LoadInputAudio(AZStd::string const& filename) -> AZStd::vector<float>
{
    auto fs{ AZ::IO::FileIOBase::GetInstance() };

    AZ::IO::HandleType fileHandle{};

    AZ::IO::Result const openInputFileResult = fs->Open(
        filename.c_str(), AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary, fileHandle);

    EXPECT_EQ(openInputFileResult.GetResultCode(), AZ::IO::ResultCode::Success);

    if (openInputFileResult != AZ::IO::ResultCode::Success)
    {
        fs->Close(fileHandle);
        return {};
    }

    AZ::u64 filesize{};
    fs->Size(fileHandle, filesize);

    auto numsamples = static_cast<int>(filesize / sizeof(float));

    AZStd::vector<float> inputaudio(numsamples);
    fs->Read(fileHandle, reinterpret_cast<char*>(inputaudio.data()), filesize, true);

    return inputaudio;
}

void SaveOutputAudio(AZStd::string const& filename, AZStd::vector<float> outputaudio)
{
    EXPECT_GT(outputaudio.size(), 0);

    auto fs{ AZ::IO::FileIOBase::GetInstance() };
    AZ::IO::HandleType fileHandle{};

    auto openResult = fs->Open(
        filename.c_str(),
        AZ::IO::OpenMode::ModeWrite | AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary,
        fileHandle);

    EXPECT_EQ(openResult.GetResultCode(), AZ::IO::ResultCode::Success);

    AZ::u64 bytesWritten{};
    fs->Write(
        fileHandle,
        reinterpret_cast<char*>(outputaudio.data()),
        outputaudio.size() * sizeof(float),
        &bytesWritten);

    EXPECT_GT(bytesWritten, 0);
    fs->Close(fileHandle);
}

TEST_F(BaseTestFixture, RunSteamAudioExample_ExportsOutputAudio)
{
    auto inputaudio = LoadInputAudio("@gemroot:SteamAudio@/Test/Assets/inputaudio.raw");

    ASSERT_GT(inputaudio.size(), 0);

    IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;

    IPLContext context{};
    iplContextCreate(&contextSettings, &context);

    IPLAudioSettings audioSettings{ TestSamplingRate, TestFrameSize };

    IPLHRTFSettings hrtfSettings;
    hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
    hrtfSettings.volume = 1.0f;

    IPLHRTF hrtf{};
    iplHRTFCreate(context, &audioSettings, &hrtfSettings, &hrtf);

    IPLBinauralEffectSettings effectSettings;
    effectSettings.hrtf = hrtf;

    IPLBinauralEffect effect{};
    iplBinauralEffectCreate(context, &audioSettings, &effectSettings, &effect);

    AZStd::vector<float> outputaudioframe(static_cast<float>(2 * TestFrameSize));
    AZStd::vector<float> outputaudio;

    auto numframes = static_cast<int>(inputaudio.size() / TestFrameSize);
    ASSERT_GT(numframes, 0);
    float* inData[] = { inputaudio.data() };

    IPLAudioBuffer inBuffer{ 1, audioSettings.frameSize, inData };

    IPLAudioBuffer outBuffer;
    iplAudioBufferAllocate(context, 2, audioSettings.frameSize, &outBuffer);

    size_t counter{};
    for (auto i = 0; i < numframes; ++i)
    {
        ++counter;
        IPLBinauralEffectParams params;
        params.direction = IPLVector3{ 1.0f, 1.0f, 1.0f };
        params.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
        params.spatialBlend = 1.0f;
        params.hrtf = hrtf;
        params.peakDelays = nullptr;

        iplBinauralEffectApply(effect, &params, &inBuffer, &outBuffer);

        iplAudioBufferInterleave(context, &outBuffer, outputaudioframe.data());

        AZStd::copy(
            AZStd::begin(outputaudioframe),
            AZStd::end(outputaudioframe),
            AZStd::back_inserter(outputaudio));

        inData[0] += audioSettings.frameSize;
    }

    EXPECT_GT(counter, 0);

    iplAudioBufferFree(context, &outBuffer);
    iplBinauralEffectRelease(&effect);
    iplHRTFRelease(&hrtf);
    iplContextRelease(&context);

    SaveOutputAudio("@assets@/outputaudio.raw", outputaudio);
}

TEST_F(BaseTestFixture, SteamAudioExample)
{
    auto fs{ AZ::IO::FileIOBase::GetInstance() };

    AZ::IO::HandleType fileHandle{};

    AZ::IO::Result const openInputFileResult = fs->Open(
        "@assets@/inputaudio.raw",
        AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary,
        fileHandle);

    ASSERT_EQ(openInputFileResult.GetResultCode(), AZ::IO::ResultCode::Success);

    AZ::u64 filesize{};
    fs->Size(fileHandle, filesize);

    auto numsamples = static_cast<int>(filesize / sizeof(float));

    AZStd::vector<float> inputaudio(numsamples);
    fs->Read(fileHandle, reinterpret_cast<char*>(inputaudio.data()), filesize, true);

    Audio::SAudioInputConfig config{};
    config.m_numChannels = 2;
    config.m_bufferSize = numsamples;
    config.m_sampleRate = TestSamplingRate;
    config.m_sourceFilename = TestInputFile;
    config.m_sourceType = Audio::AudioInputSourceType::PcmFile;
    config.m_sampleType = Audio::AudioInputSampleType::Float;
    config.m_bitsPerSample = TestBitsPerSample;

    SteamAudio::AudioInputStreaming audioSource{ config };
}

TEST_F(BaseTestFixture, WavParser_ParseWavFile_ParsesSuccessfully)
{
}
