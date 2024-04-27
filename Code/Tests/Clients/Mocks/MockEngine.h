#pragma once

#include <AzTest/AzTest.h>

#include "Engine/IEngine.h"

class MockSteamAudioEngine: public SteamAudio::IEngine
{
MOCK_METHOD0(Initialize, SteamAudio::EngineNullOutcome(void));
MOCK_METHOD0(Shutdown, SteamAudio::EngineNullOutcome(void));

};