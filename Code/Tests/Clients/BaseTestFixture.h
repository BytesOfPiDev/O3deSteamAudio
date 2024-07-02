#pragma once

#include <AzCore/UnitTest/UnitTest.h>
#include <AzTest/AzTest.h>

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Name/NameDictionary.h"
#include "AzCore/std/optional.h"
#include "AzFramework/IO/LocalFileIO.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/MaSoundEngine.h"
#include "IAudioSystemImplementation.h"

class BaseTestFixture
    : public UnitTest::TraceBusRedirector
    , public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    auto HostAudioSystemImpl() -> SteamAudio::AudioSystemImpl_steamaudio&
    {
        EXPECT_FALSE(m_audioSystemImpl.has_value())
            << "HostAudioSystemImpl should only be called once per test.";

        m_audioSystemImpl.emplace();
        return m_audioSystemImpl.value();
    };

    auto HostSoundEngine() -> SteamAudio::MaSoundEngine&
    {
        EXPECT_FALSE(m_soundEngine.has_value())
            << "HostSoundEngine should only be called once per test.";

        m_soundEngine.emplace();
        return m_soundEngine.value();
    };

    void DestroyAudioSystemImpl()
    {
        m_audioSystemImpl = AZStd::nullopt;
    }

    void DestroySoundEngine()
    {
        m_soundEngine = AZStd::nullopt;
    }

    auto GetFileIo() -> AZ::IO::FileIOBase*
    {
        return m_fileIo.get();
    }

    [[nodiscard]] auto TryGetAudioImpl() -> Audio::AudioSystemImplementationRequests*
    {
        // To simulate an EBus call, we do this instead of directly returning out member.
        auto* ptr{ Audio::AudioSystemImplementationRequestBus::FindFirstHandler() };
        EXPECT_NE(ptr, nullptr);
        return ptr;
    }

    [[nodiscard]] auto TryGetSoundEngine() -> SteamAudio::ISoundEngine*
    {
        return &m_soundEngine.value();
    }

private:
    AZ::NameDictionary m_dictionary{};
    AZ::ComponentApplication m_app{};
    AZ::Entity* m_systemEntity{};
    AZStd::optional<SteamAudio::AudioSystemImpl_steamaudio> m_audioSystemImpl{ AZStd::nullopt };
    AZStd::optional<SteamAudio::MaSoundEngine> m_soundEngine{ AZStd::nullopt };
    AZ::IO::FileIOBase* m_prevFileIo{};
    AZStd::unique_ptr<AZ::IO::LocalFileIO> m_fileIo{};
};
