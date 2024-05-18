#pragma once

#include <AzTest/AzTest.h>

#include "AzCore/Interface/Interface.h"

#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"

class MockSteamAudioEngine : public AZ::Interface<SteamAudio::ISoundEngine>::Registrar
{
public:
    MOCK_METHOD0(Initialize, SteamAudio::EngineNullOutcome(void));
    MOCK_METHOD0(Shutdown, SteamAudio::EngineNullOutcome(void));
    MOCK_METHOD1(
        RegisterAudioObject, SteamAudio::EngineNullOutcome(SteamAudio::SaGameObjectId const&));
    MOCK_METHOD1(ReportEvent, SteamAudio::EngineNullOutcome(SteamAudio::StartEventData const&));
    MOCK_CONST_METHOD0(DoNothing, void(void));
};
