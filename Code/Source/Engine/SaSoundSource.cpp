#include "SaSoundSource.h"

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Console/ILogger.h"

#include "Engine/SaSoundSourceAsset.h"
#include "Engine/SaSoundSourceBus.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    SoundSource::SoundSource(AZ::Data::Asset<SaSoundSourceData> soundSourceAsset)
        : m_soundSourceAsset{ AZStd::move(soundSourceAsset) }
    {
        if (!AZ::Data::AssetManager::IsReady())
        {
            AZLOG_ERROR("SoundSource creation failed - asset manager isn't ready")
            return;
        }

        if (!m_soundSourceAsset.QueueLoad())
        {
            AZLOG_ERROR(
                "SoundSource creation failed - unable to queue asset '%s' to load",
                m_soundSourceAsset.GetId().ToFixedString().c_str());
            return;
        }

        m_soundSourceAsset.BlockUntilLoadComplete();

        if (!m_soundSourceAsset.IsReady())
        {
            AZLOG_ERROR(
                "SoundSource creation failed - failed to load asset '%s'",
                m_soundSourceAsset.GetHint().c_str());
            return;
        }

        auto* soundSrc{ m_soundSourceAsset.GetAs<SaSoundSourceData>() };
        m_name = soundSrc->GetSoundSourceName();

        AZLOG_INFO("SoundSource is being created with name '%s'", m_name.GetCStr());

        if (m_name.IsEmpty())
        {
            AZLOG_ERROR("SoundSource creation failed - empty name");
            return;
        }

        Register();
    }

    SoundSource::~SoundSource()
    {
        if (!m_soundSourceAsset || m_name.IsEmpty())
        {
            return;
        }

        SaSoundSourceRequestBus::Handler::BusDisconnect();

        auto const result = ma_resource_manager_unregister_data(
            ma_engine_get_resource_manager(Util::GetMaEngine()), m_name.GetCStr());

        AZ_Error(
            "SoundSource",
            result == MA_SUCCESS,
            "Failed to unregister asset '%s' with name '%s'",
            m_soundSourceAsset.GetHint().c_str(),
            m_name.GetCStr());

        AZLOG_INFO("SoundSourcce '%s' unregistered", m_name.GetCStr());
    }

    void SoundSource::Register()
    {
        if (!m_soundSourceAsset)
        {
            AZLOG_ERROR("SoundSource registration failed - null asset data");
            return;
        }

        if (!m_soundSourceAsset->GetSoundAsset().IsReady())
        {
            AZLOG_ERROR("SoundSource registration failed - sound is not ready");
            return;
        }

        AZLOG_INFO(
            "SoundSource registering '%s' with miniaudio resource manager, size '%lu'",
            m_name.GetCStr(),
            m_soundSourceAsset->GetSoundAsset()->m_audioData.size());

        auto const result = ma_resource_manager_register_encoded_data(
            ma_engine_get_resource_manager(Util::GetMaEngine()),
            m_soundSourceAsset->GetSoundSourceName().GetCStr(),
            m_soundSourceAsset->GetSoundAsset()->m_audioData.data(),
            m_soundSourceAsset->GetSoundAsset()->m_audioData.size());

        if (result != MA_SUCCESS)
        {
            m_soundSourceAsset.Reset();
            m_name = AZ::Name{};

            AZLOG_INFO(
                "SoundSource registration failed - '%s' could not be registered with miniaudio as "
                "'%s'",
                m_soundSourceAsset.GetHint().c_str(),
                m_name.GetCStr());

            return;
        }

        SaSoundSourceRequestBus::Handler::BusConnect(m_name);
        AZLOG_INFO(
            "SoundSource '%s' registered as '%s'",
            m_soundSourceAsset.GetHint().c_str(),
            m_name.GetCStr());
    }

    auto SoundSource::CreateInstance() const -> AZStd::any
    {
        if (!SteamAudio::SaSoundSourceRequestBus::Handler::BusIsConnectedId(m_name))
        {
            return {};
        }
        auto sound{ AZStd::make_any<ma_sound>() };
        auto const soundInitResult{ ma_sound_init_from_file(
            Util::GetMaEngine(),
            m_name.GetCStr(),
            0,
            nullptr,
            nullptr,
            &AZStd::any_cast<ma_sound&>(sound)) };

        if (soundInitResult != MA_SUCCESS)
        {
            AZLOG_ERROR("Failed to create SoundInstance for '%s'", m_name.GetCStr());
            return {};
        }

        AZLOG_INFO("SoundSource created instance of '%s'", m_name.GetCStr());

        return AZStd::move(sound);
    }
}  // namespace SteamAudio
