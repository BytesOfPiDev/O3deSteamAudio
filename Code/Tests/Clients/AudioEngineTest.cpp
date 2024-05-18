#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "Clients/SoundEngineTestFixture.h"
#include "Engine/Id.h"
#include "Engine/Parameters.h"
#include "Engine/SoundEngine.h"
#include "IAudioSystem.h"

using ::testing::Return;

TEST_F(SoundEngineTestFixture, SANITY_CHECK)
{
}

TEST(SoundEngine, Unitialiazed_Initialize_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    auto const initEngineOutcome{ engine.Initialize() };
    EXPECT_TRUE(initEngineOutcome.IsSuccess()) << initEngineOutcome.GetError().c_str();
}

TEST(SoundEngine, Initialized_ReportEvent_PassEmptyEventName_ReturnsFailure)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();

    static auto constexpr nonExistentEventName{ "" };
    auto const nonExistentEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ nonExistentEventName } } };

    auto const reportEventOutcome{ engine.ReportEvent(nonExistentEventData) };
    EXPECT_TRUE(!reportEventOutcome.IsSuccess()) << reportEventOutcome.GetError().c_str();
}

TEST(SoundEngine, Initialized_ReportEvent_PassNonExistentEventName_ReturnsFailure)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();

    [&engine]()
    {
        static auto constexpr nonExistentEventName{ "*" };
        auto const nonExistentStartEventData{ SteamAudio::StartEventData{
            Audio::AudioStringToID<SteamAudio::SaEventId>(nonExistentEventName),
            SteamAudio::SaGameObjectId{},
            SteamAudio::SaId{ nonExistentEventName } } };

        auto const reportEventOutcome{ engine.ReportEvent(nonExistentStartEventData) };
        EXPECT_TRUE(!reportEventOutcome.IsSuccess()) << reportEventOutcome.GetError().c_str();
    }();
}

TEST(SoundEngineNativeEvents, DoNothing_ReportEvent_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();
    auto const doNothingStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::DoNothingEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ SteamAudio::Events::DoNothingEventName } } };

    auto const reportDoNothingEventOutcome{ engine.ReportEvent(doNothingStartEventData) };
    EXPECT_TRUE(reportDoNothingEventOutcome.IsSuccess())
        << reportDoNothingEventOutcome.GetError().c_str();
}

TEST(SoundEngineNativeEvents, MuteAll_ReportEvent_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();
    auto const muteAllStartEventName{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::MuteAllEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ SteamAudio::Events::MuteAllEventName } } };

    auto const reportMuteAllEventOutcome{ engine.ReportEvent(muteAllStartEventName) };
    EXPECT_TRUE(reportMuteAllEventOutcome.IsSuccess())
        << reportMuteAllEventOutcome.GetError().c_str();
}

TEST(SoundEngineNativeEvents, UnmuteAll_ReportEvent_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();
    auto const unMuteAllStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::UnmuteAllEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ SteamAudio::Events::UnmuteAllEventName } } };

    auto const reportUnmuteAllEventOutcome{ engine.ReportEvent(unMuteAllStartEventData) };
    EXPECT_TRUE(reportUnmuteAllEventOutcome.IsSuccess())
        << reportUnmuteAllEventOutcome.GetError().c_str();
}

TEST(SoundEngineNativeEvents, GetFocus_ReportEvent_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();
    auto const getFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::GetFocusEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ SteamAudio::Events::GetFocusEventName } } };

    auto const reportGetFocusEventOutcome{ engine.ReportEvent(getFocusStartEventData) };
    EXPECT_TRUE(reportGetFocusEventOutcome.IsSuccess())
        << reportGetFocusEventOutcome.GetError().c_str();
}

TEST(SoundEngineNativeEvents, LoseFocus_ReportEvent_ReturnsSuccess)
{
    SteamAudio::SteamAudioEngine engine;
    engine.Initialize();
    auto const loseFocusStartEventData{ SteamAudio::StartEventData{
        Audio::AudioStringToID<SteamAudio::SaEventId>(SteamAudio::Events::LoseFocusEventName),
        SteamAudio::SaGameObjectId{},
        SteamAudio::SaId{ SteamAudio::Events::LoseFocusEventName } } };

    auto const reportLoseFocusEventOutcome{ engine.ReportEvent(loseFocusStartEventData) };
    EXPECT_TRUE(reportLoseFocusEventOutcome.IsSuccess())
        << reportLoseFocusEventOutcome.GetError().c_str();
}
