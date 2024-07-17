#pragma once

#include "Clients/BaseTestFixture.h"

class SoundEngineTestFixture : public BaseTestFixture
{
public:
    auto GetEngine() -> SteamAudio::MaSoundEngine&
    {
        return m_engine;
    }

protected:
    void SetUp() override
    {
        BaseTestFixture::SetUp();
        m_engine.Initialize();
    }
    void TearDown() override
    {
        m_engine.Shutdown();
        BaseTestFixture::TearDown();
    }

private:
    SteamAudio::MaSoundEngine m_engine;
};
