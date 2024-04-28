#include <AzTest/AzTest.h>

#include "Clients/AudioSystemImplTestFixture.h"
#include "Clients/Mocks/MockSoundEngine.h"
#include "Engine/ATLEntities_steamaudio.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

using ::testing::Return;

TEST_F(AudioSystemImplTestFixture, SANITY_CHECK)
{
    EXPECT_TRUE(AZ::Interface<Audio::IAudioSystem>::Get() != nullptr);
}

TEST_F(AudioSystemImplTestFixture, CreateAutoImplWithNoEngine_CallInitialize_ReturnsFalse)
{
    SteamAudio::AudioSystemImpl_steamaudio impl{ "test" };
    EXPECT_NE(impl.Initialize(), Audio::EAudioRequestStatus::Success);
}

TEST_F(AudioSystemImplTestFixture, CreateAudioImpl_CallInitialize_ReturnsSuccess)
{
    auto impl{ SteamAudio::AudioSystemImpl_steamaudio("test") };
    auto soundEngine{ MockSteamAudioEngine{} };

    EXPECT_CALL(soundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_EQ(impl.Initialize(), Audio::EAudioRequestStatus::Success);
}

TEST_F(
    AudioSystemImplTestFixture,
    InitializedAudioImpl_RegisterAudioObjectWithValidArgs_ReturnsSuccess)
{
    auto impl{ SteamAudio::AudioSystemImpl_steamaudio("test") };
    impl.Initialize();

    static constexpr auto* validObjectName{ "my_leg" };
    SteamAudio::SATLAudioObjectData_steamaudio validObjectData{};

    EXPECT_EQ(
        impl.RegisterAudioObject(&validObjectData, validObjectName),
        Audio::EAudioRequestStatus::Success);
}
