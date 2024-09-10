#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/UnitTest/UnitTest.h"
#include "Clients/BaseTestFixture.h"
#include "Clients/SoundEngineTestFixture.h"
#include "Engine/Id.h"
#include "Engine/MaSoundEngine.h"
#include "Engine/Parameters.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

using ::testing::Return;

class SoundEngineNativeEvents : public BaseTestFixture
{
};

TEST_F(SoundEngineTestFixture, SANITY_CHECK)
{
}

TEST_F(SoundEngineTestFixture, Initialized_ReportEvent_PassEmptyEventName_ReturnsFailure)
{
    static auto constexpr nonExistentEventName{ "" };
    auto const nonExistentEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
        SteamAudio::SaGameObjectId{} } };

    AZ_TEST_START_TRACE_SUPPRESSION;
    auto const eventInstanceId{ GetEngine().ReportEvent(nonExistentEventData) };
    AZ_TEST_STOP_TRACE_SUPPRESSION(1);
    EXPECT_EQ(eventInstanceId, SteamAudio::InvalidEventInstanceId);
    EXPECT_TRUE(GetEngine().Shutdown().IsSuccess());
}

TEST_F(SoundEngineTestFixture, Initialized_ReportEvent_PassNonExistentEventName_ReturnsFailure)
{
    static auto constexpr nonExistentEventName{ "*" };
    auto const nonExistentStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    AZ_TEST_START_TRACE_SUPPRESSION;
    auto const eventInstanceId{ GetEngine().ReportEvent(nonExistentStartEventData) };
    AZ_TEST_STOP_TRACE_SUPPRESSION(2);

    EXPECT_EQ(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, DoNothing_ReportEvent_ReturnsSuccess)
{
    SteamAudio::MaSoundEngine engine;
    auto const engineInitOutcome{ engine.Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const doNothingStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::DoNothingEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ engine.ReportEvent(doNothingStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, MuteAll_ReportEvent_ReturnsSuccess)
{
    SteamAudio::MaSoundEngine engine;
    auto const engineInitOutcome{ engine.Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const muteAllStartEventName{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::MuteAllEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ engine.ReportEvent(muteAllStartEventName) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, UnmuteAll_ReportEvent_ReturnsSuccess)
{
    SteamAudio::MaSoundEngine engine;
    auto const engineInitOutcome{ engine.Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const unMuteAllStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::UnmuteAllEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ engine.ReportEvent(unMuteAllStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, GetFocus_ReportEvent_ReturnsSuccess)
{
    SteamAudio::MaSoundEngine engine;
    auto const engineInitOutcome{ engine.Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const getFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::GetFocusEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ engine.ReportEvent(getFocusStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}

TEST_F(SoundEngineNativeEvents, LoseFocus_ReportEvent_ReturnsSuccess)
{
    SteamAudio::MaSoundEngine engine;
    auto engineInitOutcome{ engine.Initialize() };
    EXPECT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();

    auto const loseFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::LoseFocusEventName),
        SteamAudio::SaGameObjectId{ GLOBAL_AUDIO_OBJECT_ID } } };

    auto const eventInstanceId{ engine.ReportEvent(loseFocusStartEventData) };
    EXPECT_NE(eventInstanceId, SteamAudio::InvalidEventInstanceId);
}
