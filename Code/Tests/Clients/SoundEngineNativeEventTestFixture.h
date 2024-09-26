#pragma once

#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "Clients/BaseTestFixture.h"
#include "Engine/AudioObject.h"
#include "Engine/MaSoundEngine.h"
#include "IAudioInterfacesCommonData.h"

class SoundEngineNativeEvents : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();
        m_engine.emplace();
        m_globalObject.emplace(GLOBAL_AUDIO_OBJECT_ID);
    }

    void TearDown() override
    {
        m_globalObject.reset();
        m_engine.reset();
        BaseTestFixture::TearDown();
    }

    auto GetSoundEngine() const -> SteamAudio::MaSoundEngine const&
    {
        return m_engine.value();
    }

    auto GetSoundEngine() -> SteamAudio::MaSoundEngine&
    {
        return m_engine.value();
    }

private:
    AZStd::optional<SteamAudio::MaSoundEngine> m_engine{ AZStd::nullopt };
    AZStd::optional<SteamAudio::SaAudioObject> m_globalObject{ AZStd::nullopt };
};
