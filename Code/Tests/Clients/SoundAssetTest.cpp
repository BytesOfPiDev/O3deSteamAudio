#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/IO/FileIO.h"

#include "Clients/BaseTestFixture.h"
#include "Engine/SoundAsset.h"
#include "SteamAudio/dr_wav.h"

static constexpr auto TestFile1Path{ "@assets@/01_welcome_01.wav" };
static constexpr auto WavTestFile1SampleRate{ 44100u };
static constexpr auto WavTestFile1NumChannels{ 2u };
static constexpr auto WavTestFile1NumSamples{ 39936u };

struct SoundAssetTests : BaseTestFixture
{
};

struct WavFileTestFixture : BaseTestFixture
{
};

TEST_F(SoundAssetTests, SANITY_CHECK)
{
    SteamAudio::SaSoundAsset asset{};
}

TEST_F(WavFileTestFixture, WavSourceFile_Decode_ReturnsCorrectSettings)
{
    EXPECT_NE(AZ::IO::FileIOBase::GetInstance(), nullptr);

    auto resolvePathResult = AZ::IO::FileIOBase::GetInstance()->ResolvePath(TestFile1Path);
    EXPECT_TRUE(resolvePathResult.has_value());

    drwav wav;

    bool initWavFileSuccess{ static_cast<bool>(
        drwav_init_file(&wav, resolvePathResult->c_str(), nullptr)) };

    EXPECT_TRUE(initWavFileSuccess)
        << "Failed to init wav file with path: %s " << resolvePathResult->c_str();
    EXPECT_GT(wav.totalPCMFrameCount, 0);

    auto decodedData{ AZStd::vector<float>(wav.totalPCMFrameCount * wav.channels * sizeof(float)) };

    auto const numSamplesDecoded =
        drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, decodedData.data());

    EXPECT_EQ(wav.channels, WavTestFile1NumChannels);

    EXPECT_EQ(wav.sampleRate, WavTestFile1SampleRate);
    EXPECT_EQ(numSamplesDecoded, WavTestFile1NumSamples);

    drwav_uninit(&wav);
    decodedData.clear();
}
