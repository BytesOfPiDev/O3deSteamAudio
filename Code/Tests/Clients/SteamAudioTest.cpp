#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/OpenMode.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "Clients/BaseTestFixture.h"
#include "Clients/Mocks/MockSoundEngine.h"
#include "Engine/ATLEntities_steamaudio.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "IAudioInterfacesCommonData.h"

using ::testing::Return;

TEST_F(BaseTestFixture, SANITY_CHECK)
{
}

TEST_F(BaseTestFixture, CreateImplWithNoEngine_CallInitialize_ReturnsFailure)
{
    auto& audioSystemImpl{ HostAudioSystemImpl() };

    AZ_TEST_START_TRACE_SUPPRESSION;
    EXPECT_EQ(audioSystemImpl.Initialize(), Audio::EAudioRequestStatus::Failure);
    AZ_TEST_STOP_TRACE_SUPPRESSION(1);
}

TEST_F(BaseTestFixture, CreateAudioImpl_CallInitialize_ReturnsSuccess)
{
    auto soundEngine{ MockSteamAudioEngine{} };

    EXPECT_CALL(soundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));

    auto& impl{ HostAudioSystemImpl() };
    EXPECT_EQ(impl.Initialize(), Audio::EAudioRequestStatus::Success);
}

TEST_F(BaseTestFixture, InitializedAudioImpl_RegisterAudioObjectWithValidArgs_ReturnsSuccess)
{
    MockSteamAudioEngine mockSoundEngine{};
    EXPECT_CALL(mockSoundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, RegisterAudioObject).Times(1).WillOnce(Return(AZ::Success()));

    auto impl{ SteamAudio::AudioSystemImpl_steamaudio() };
    impl.Initialize();

    static constexpr auto* validObjectName{ "my_leg" };
    SteamAudio::SATLAudioObjectData_steamaudio validObjectData{};

    EXPECT_EQ(
        impl.RegisterAudioObject(&validObjectData, validObjectName),
        Audio::EAudioRequestStatus::Success);
}

TEST_F(BaseTestFixture, InitializedAudioImpl_ActivateValidTrigger_ReturnsSuccess)
{
    MockSteamAudioEngine mockSoundEngine{};
    EXPECT_CALL(mockSoundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, StartEvent).Times(1);

    SteamAudio::SATLAudioObjectData_steamaudio objData{
        SteamAudio::SaGameObjectId{ AZ::Entity::MakeId() }, false
    };

    SteamAudio::SATLTriggerImplData_steamaudio triggerData{};

    SteamAudio::SATLEventData_steamaudio eventData{};

    [[maybe_unused]] Audio::SATLSourceData sourceData{};

    [[maybe_unused]] auto& impl{ HostAudioSystemImpl() };
    impl.Initialize();
    impl.ActivateTrigger(&objData, &triggerData, &eventData, &sourceData);
}
