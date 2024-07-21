
#include "Clients/BaseTestFixture.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/MaSoundEngine.h"
#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

class SoundResourceManagerTestFixture : public BaseTestFixture
{
    void SetUp() override
    {
        BaseTestFixture::SetUp();
        m_engine.emplace();
        m_impl.emplace();
    }

    void TearDown() override
    {
        m_impl = AZStd::nullopt;
        m_engine = AZStd::nullopt;

        BaseTestFixture::TearDown();
    }

private:
    AZStd::optional<SteamAudio::AudioSystemImpl_steamaudio> m_impl;
    AZStd::optional<SteamAudio::MaSoundEngine> m_engine;
};

TEST_F(SoundResourceManagerTestFixture, DISABLED_SANITY_CHECK)
{
}
