#pragma once

#include "Clients/BaseTestFixture.h"

class AudioImplTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();

        [[maybe_unused]] auto& audioImpl{ HostAudioSystemImpl() };
        [[maybe_unused]] auto& soundEngine{ HostSoundEngine() };
        audioImpl.Initialize();
    }

    void TearDown() override
    {
        BaseTestFixture::TearDown();
    }
};
