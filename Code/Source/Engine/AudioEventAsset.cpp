#include "Engine/AudioEventAsset.h"

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "Engine/ISoundEngine.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

#include "Engine/AudioEventAsset.h"
#include "Engine/Id.h"
#include "Engine/ResourceManager.h"
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
            CreateSound(PathToSoundName(asset.GetHint()), &m_sound);
        }

        PlaySoundFunc(PlaySoundFunc const& other)
            : m_asset{ other.m_asset }
        {
            CopySound(&other.m_sound, &m_sound);
        }

        PlaySoundFunc(PlaySoundFunc&& other)
            : m_asset(AZStd::move(other.m_asset))
        {
            other.m_asset = {};
            CopySound(&other.m_sound, &m_sound);
            ma_sound_uninit(&other.m_sound);
        }

        ~PlaySoundFunc() = default;

        auto operator=(PlaySoundFunc const& other) -> PlaySoundFunc&
        {
            m_asset = other.m_asset;
            ma_sound_uninit(&m_sound);
            CopySound(&other.m_sound, &m_sound);

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
                ->Version(2)
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Field("Name", &SaEventAsset::m_name)
                ->Field("EventId", &SaEventAsset::m_eventId)
                ->Field("Sound", &SaEventAsset::m_sound);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<SaEventAsset>(TYPEINFO_Name(), "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute("AutoExpand", true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SaEventAsset::m_name, "Name", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &SaEventAsset::UpdateId)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SaEventAsset::m_sound, "Sound", "");
            }
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
        m_eventId = Audio::AudioStringToID<SaEventId>(m_name.c_str());
    }

    void SaEventAsset::UpdateId()
    {
        SetEventId(m_name);
    }

    auto SaEventAsset::CreateInstance() const -> AZStd::unique_ptr<SaEvent>
    {
        return m_setupFunc ? m_setupFunc(m_assetId)
                           : AZStd::make_unique<SaEvent>(
                                 PlaySoundFunc(m_sound),
                                 [](SaGameObjectId) -> void
                                 {
                                 });
    }

}  // namespace SteamAudio
