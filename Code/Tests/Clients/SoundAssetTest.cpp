#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "Clients/BaseTestFixture.h"
#include "Engine/SaSoundAsset.h"

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
