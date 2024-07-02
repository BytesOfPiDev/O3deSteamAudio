#pragma once

#include "Clients/BaseTestFixture.h"

class SoundEngineTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();
        HostAudioSystemImpl();
    }

    void TearDown() override
    {
        DestroyAudioSystemImpl();
        BaseTestFixture::TearDown();
    }
};
