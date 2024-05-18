#pragma once

#include <AzTest/AzTest.h>

#include "AzCore/Name/Name.h"
#include "Engine/ResourceManager.h"
#include "Engine/SoundAsset.h"

class MockSoundResourceManager : public SteamAudio::SoundResourceManagerRequestBus::Handler
{
public:
    MOCK_METHOD2(RegisterSound, bool(AZ::Data::Asset<SteamAudio::SaSoundAsset>, AZ::Name));
    MOCK_METHOD1(UnregisterSound, bool(AZ::Name));
};
