#pragma once

#include "Clients/BaseTestFixture.h"
#include "Engine/MaSoundEngine.h"

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
        auto const engineInitOutcome{ m_engine.Initialize() };
        ASSERT_TRUE(engineInitOutcome.IsSuccess()) << engineInitOutcome.GetError().c_str();
    }
    void TearDown() override
    {
        auto const engineShutdownOutcome{ m_engine.Shutdown() };
        ASSERT_TRUE(engineShutdownOutcome.IsSuccess()) << engineShutdownOutcome.GetError().c_str();
        BaseTestFixture::TearDown();
    }

private:
    SteamAudio::MaSoundEngine m_engine;
};
