#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/UnitTest/UnitTest.h"

#include "Clients/SoundEngineNativeEventTestFixture.h"
#include "Clients/SoundEngineTestFixture.h"
#include "Engine/Id.h"
#include "Engine/MaSoundEngine.h"
#include "Engine/Parameters.h"
#include "Engine/SaAudioObjectBus.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

using ::testing::Return;

TEST_F(SoundEngineTestFixture, SANITY_CHECK)
{
}

TEST_F(SoundEngineTestFixture, Initialized_ReportEvent_PassEmptyEventName_ReturnsFailure)
{
    static auto constexpr nonExistentEventName{ "" };
    auto const nonExistentEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
        SteamAudio::SaAudioObjectId{} } };

    AZ_TEST_START_TRACE_SUPPRESSION;
    auto const eventInstanceId{ GetEngine().PostEvent(nonExistentEventData) };
    AZ_TEST_STOP_TRACE_SUPPRESSION(1);
    EXPECT_EQ(eventInstanceId, SteamAudio::InvalidEventInstanceId);
    EXPECT_TRUE(GetEngine().Shutdown().IsSuccess());
}

TEST_F(SoundEngineTestFixture, Initialized_ReportEvent_PassNonExistentEventName_ReturnsFailure)
{
    static auto constexpr nonExistentEventName{ "*" };
    auto const nonExistentStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
        SteamAudio::SaAudioObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    AZ_TEST_START_TRACE_SUPPRESSION;
    auto const eventInstanceId{ GetEngine().PostEvent(nonExistentStartEventData) };
    AZ_TEST_STOP_TRACE_SUPPRESSION(2);

    EXPECT_EQ(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, DoNothing_ReportEvent_ReturnsSuccess)
{
    auto const engineInitOutcome{ GetSoundEngine().Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const doNothingStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::DoNothingEventName),
        SteamAudio::GlobalSaGameObjectId } };

    auto const eventInstanceId{ GetSoundEngine().PostEvent(doNothingStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, MuteAll_ReportEvent_ReturnsSuccess)
{
    auto const engineInitOutcome{ GetSoundEngine().Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const muteAllStartEventName{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::MuteAllEventName),
        SteamAudio::GlobalSaGameObjectId } };

    auto const eventInstanceId{ GetSoundEngine().PostEvent(muteAllStartEventName) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, UnmuteAll_ReportEvent_ReturnsSuccess)
{
    auto const engineInitOutcome{ GetSoundEngine().Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const unMuteAllStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::UnmuteAllEventName),
        SteamAudio::SaAudioObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ GetSoundEngine().PostEvent(unMuteAllStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, GetFocus_ReportEvent_ReturnsSuccess)
{
    auto const engineInitOutcome{ GetSoundEngine().Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const getFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::GetFocusEventName),
        SteamAudio::SaAudioObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ GetSoundEngine().PostEvent(getFocusStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, LoseFocus_ReportEvent_ReturnsSuccess)
{
    auto engineInitOutcome{ GetSoundEngine().Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const loseFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::LoseFocusEventName),
        SteamAudio::SaAudioObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ GetSoundEngine().PostEvent(loseFocusStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}
