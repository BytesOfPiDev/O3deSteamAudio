#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/UnitTest/UnitTest.h"
#include "Clients/AudioImplTestFixture.h"
#include "IAudioInterfacesCommonData.h"

#include "Clients/BaseTestFixture.h"
#include "Clients/Mocks/MockSoundEngine.h"
#include "Engine/ATLEntities_steamaudio.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/Id.h"
#include "IAudioSystem.h"

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

TEST_F(BaseTestFixture, InitializeAudioImpl_ActivateInvalidTriggerUsingValidImplData_ReturnsFailure)
{
    MockSteamAudioEngine mockSoundEngine{};
    EXPECT_CALL(mockSoundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, ReportEvent)
        .Times(1)
        .WillOnce(Return(AZ::Failure("Event does not exist")));

    auto& impl{ HostAudioSystemImpl() };
    impl.Initialize();

    SteamAudio::SATLAudioObjectData_steamaudio objData{
        SteamAudio::SaGameObjectId{ AZ::Entity::MakeId() }, false
    };

    SteamAudio::SATLTriggerImplData_steamaudio const triggerData{};

    static constexpr AZStd::string_view eventName{ "this_trigger_does_not_exist" };

    auto const atlEventId{ Audio::AudioStringToID<Audio::TAudioEventID>(eventName.data()) };

    SteamAudio::SATLEventData_steamaudio eventData{ atlEventId };

    Audio::SATLSourceData const sourceData{};

    AZ_TEST_START_TRACE_SUPPRESSION;
    EXPECT_NE(
        impl.ActivateTrigger(&objData, &triggerData, &eventData, &sourceData),
        Audio::EAudioRequestStatus::Success);
    AZ_TEST_STOP_TRACE_SUPPRESSION(1);

    EXPECT_EQ(eventData.GetEventState(), Audio::EAudioEventState::eAES_NONE);
}

TEST_F(AudioImplTestFixture, Initialized_ActivateDoNothingEvent_ReturnsSuccess)
{
    SteamAudio::SATLAudioObjectData_steamaudio objData{
        SteamAudio::SaGameObjectId{ AZ::Entity::MakeId() }, false
    };

    SteamAudio::SATLTriggerImplData_steamaudio const triggerData{
        SteamAudio::Events::DoNothingEventName
    };

    auto const atlEventId{ Audio::AudioStringToID<Audio::TAudioEventID>(
        SteamAudio::Events::DoNothingEventName) };

    SteamAudio::SATLEventData_steamaudio eventData{ atlEventId };

    Audio::SATLSourceData const sourceData{};

    auto* impl{ TryGetAudioImpl() };
    EXPECT_NE(impl, nullptr);

    EXPECT_EQ(
        impl->ActivateTrigger(&objData, &triggerData, &eventData, &sourceData),
        Audio::EAudioRequestStatus::Success);

    EXPECT_NE(eventData.GetEventState(), Audio::EAudioEventState::eAES_NONE);
}
