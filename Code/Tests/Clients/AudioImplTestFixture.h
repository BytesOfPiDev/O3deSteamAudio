#pragma once

#include "IAudioInterfacesCommonData.h"

#include "Clients/BaseTestFixture.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/MaSoundEngine.h"

class AudioImplTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();

        m_engine.emplace();
        m_impl.emplace();

        AZ_TEST_ASSERT(m_impl->Initialize() == Audio::EAudioRequestStatus::Success);
    }

    void TearDown() override
    {
        m_impl.reset();

        BaseTestFixture::TearDown();
    }

private:
    AZStd::optional<SteamAudio::MaSoundEngine> m_engine{ AZStd::nullopt };
    AZStd::optional<SteamAudio::AudioSystemImpl_steamaudio> m_impl{ AZStd::nullopt };
};
