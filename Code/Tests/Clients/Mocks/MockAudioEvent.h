#pragma once

#include <AzTest/AzTest.h>

#include "Engine/AudioEvent.h"

class MockAudioEvent : public SteamAudio::AudioEvent
{
public:
    MOCK_METHOD1(Update, void(float));
    MOCK_METHOD0(GetEventState, SteamAudio::SaAudioEventState(void));
};
