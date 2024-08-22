#include "Engine/AudioEventAsset.h"

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/PlatformDef.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

#include "Engine/Id.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/SteamAudioTypeIds.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    using Audio::TAudioEventID;

    AZ_RTTI_NO_TYPE_INFO_IMPL(SaEventAsset, AZ::Data::AssetData);
    AZ_TYPE_INFO_WITH_NAME_IMPL(SaEventAsset, "SteamAudio Event Asset", SaEventAssetTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(SaEventAsset, Audio::AudioImplAllocator);

    struct PlaySoundFunc
    {
        PlaySoundFunc()
            : m_sound{}
        {
        }

        PlaySoundFunc(AZ::Data::Asset<SaSoundAsset> const& asset)
            : m_asset{ asset }
        {
            if (!asset)
            {
                AZ_Warning(
                    "PlaySoundFunc",
                    false,
                    "Received empty sound asset. No sound will be registered.");
                return;
            }

            ma_resource_manager_register_encoded_data(
                ma_engine_get_resource_manager(Util::GetMaEngine()),
                AZ::IO::Path{ m_asset.GetHint() }.Stem().String().c_str(),
                m_asset->m_data.data(),
                m_asset->m_data.size());
        }

        PlaySoundFunc(PlaySoundFunc const& other)
            : m_asset{ other.m_asset }
        {
        }

        PlaySoundFunc(PlaySoundFunc&& other)
            : m_asset(AZStd::move(other.m_asset))
        {
            other.m_asset = {};

            // Miniaudio doesn't check for null data source, so we have to check
            if (!ma_data_source_get_current(&other.m_sound))
            {
                AZ_Error(
                    AZ_FUNCTION_SIGNATURE,
                    !ma_data_source_get_current(&other.m_sound),
                    "Incoming sound is null.");
                return;
            }

            ma_sound_uninit(&other.m_sound);
        }

        ~PlaySoundFunc() = default;

        auto operator=(PlaySoundFunc const& other) -> PlaySoundFunc&
        {
            m_asset = other.m_asset;
            ma_sound_uninit(&m_sound);

            return *this;
        }
        auto operator=(PlaySoundFunc&& other) -> PlaySoundFunc&
        {
            ma_sound_uninit(&m_sound);
            ma_sound_uninit(&other.m_sound);

            m_asset = AZStd::move(other.m_asset);
            other.m_asset = {};

            AZ::IO::PathView const soundName{ m_asset.GetHint().c_str() };

            auto* maEngine{ Util::GetMaEngine() };
            (maEngine != nullptr) &&
                ma_sound_init_from_file(
                    maEngine, soundName.Stem().String().c_str(), 0, nullptr, nullptr, &m_sound);

            return *this;
        }

        void operator()(SaGameObjectId)
        {
            AZLOG(LOG_SaEvent, "PlaySoundFunc call. Asset: %s", m_asset.GetHint().c_str());

            ma_sound_init_from_file(
                Util::GetMaEngine(),
                AZ::IO::Path{ m_asset.GetHint() }.Stem().String().c_str(),
                0,
                nullptr,
                nullptr,
                &m_sound);

            ma_sound_set_volume(&m_sound, 1.0f);
            ma_sound_set_looping(&m_sound, true);
            ma_sound_start(&m_sound);
        }

        AZ::Data::Asset<SaSoundAsset> m_asset{};
        ma_sound m_sound{};
    };

    void SaEventAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SaEventAsset, AZ::Data::AssetData>()
                ->Version(4)
                ->Field("Name", &SaEventAsset::m_name)
                ->Field("EventId", &SaEventAsset::m_eventId)
                ->Field("Sound", &SaEventAsset::m_soundAsset);
        }
    }

    SaEventAsset::SaEventAsset() = default;

    SaEventAsset::SaEventAsset(AudioEventName eventName)
    {
        SetEventName(AZStd::move(eventName));
        eventName = "";
    }

    SaEventAsset::SaEventAsset(AudioEventName eventName, SetupFunc setupFunc)
        : m_setupFunc{ AZStd::move(setupFunc) }
    {
        SetEventName(AZStd::move(eventName));
    }

    SaEventAsset::~SaEventAsset() = default;

    void SaEventAsset::SetEventName(AudioEventName eventName)
    {
        m_name = AZStd::move(eventName);
        eventName = "";
        UpdateId();
    }

    void SaEventAsset::UpdateId()
    {
        m_eventId = Audio::AudioStringToID<SaEventId>(m_name.c_str());
    }

    auto SaEventAsset::CreateInstance() const -> AZStd::unique_ptr<SaEvent>
    {
        if (m_soundAsset)
        {
            auto const assetHint{ AZ::IO::PathView{ m_soundAsset.GetHint() } };
            auto const soundName{ assetHint.Stem().String() };
            AZLOG(
                LOG_SaEventAsset,
                "%s: registering sound with name '%s'",
                AZ_FUNCTION_SIGNATURE,
                soundName.c_str());
        }

        return m_setupFunc ? m_setupFunc(m_assetId)
                           : AZStd::make_unique<SaEvent>(
                                 PlaySoundFunc(m_soundAsset),
                                 [&](SaGameObjectId) -> void
                                 {
                                     AZLOG(
                                         LOG_SaEventAsset,
                                         "Executing event %s | %llu",
                                         m_name.c_str(),
                                         static_cast<AZ::u64>(m_eventId));
                                 });
    }

    void SaEventAsset::SetSound(AZ::Data::Asset<SaSoundAsset> const soundAsset)
    {
        m_soundAsset = AZStd::move(soundAsset);
    };
}  // namespace SteamAudio
