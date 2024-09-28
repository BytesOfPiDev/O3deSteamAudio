#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "Clients/BaseTestFixture.h"
#include "Engine/SaSoundSource.h"
#include "Engine/SaSoundSourceAsset.h"

class SoundSourceTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(SoundSourceTestFixture, SANITY_CHECK)
{
}

TEST_F(SoundSourceTestFixture, Construction_WithNullAssetId_CallIsBusConnected_ReturnsFalse)
{
    SteamAudio::SoundSource soundSrc{ {} };
    EXPECT_FALSE(soundSrc.BusIsConnected());
}

TEST_F(SoundSourceTestFixture, Construction_WithBadAssetId_CallIsBusConnected_ReturnsFalse)
{
    SteamAudio::SoundSource soundSrc{ SteamAudio::SaSoundSourceAssetPtr{
        AZ::Data::AssetId{ AZ::Uuid::CreateRandom() },
        AZ::Data::AssetType{ AZ::AzTypeInfo<SteamAudio::SaSoundSourceData>::Uuid() } } };
    EXPECT_FALSE(soundSrc.BusIsConnected());
}

TEST_F(SoundSourceTestFixture, DISABLED_Construction_WithGoodAssetId_CallIsBusConnected_ReturnsTrue)
{
    EXPECT_TRUE(false);
}
