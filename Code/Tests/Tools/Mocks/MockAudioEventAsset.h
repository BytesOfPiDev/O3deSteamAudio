#pragma once

#include "Engine/AudioEventAsset.h"
#include "gmock/gmock-generated-function-mockers.h"

class MockAudioEventAsset : public SteamAudio::SaEventAsset
{
public:
    MOCK_CONST_METHOD0(GetEventName, SteamAudio::SaEventAsset::AudioEventName(void));
};
