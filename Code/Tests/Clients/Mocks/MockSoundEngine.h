#pragma once

#include <AzTest/AzTest.h>

#include "AzCore/Interface/Interface.h"

#include "Engine/ISoundEngine.h"

class MockSteamAudioEngine : public AZ::Interface<SteamAudio::IEngine>::Registrar
{
public:
    MOCK_METHOD0(Initialize, SteamAudio::EngineNullOutcome(void));
    MOCK_METHOD0(Shutdown, SteamAudio::EngineNullOutcome(void));
};
