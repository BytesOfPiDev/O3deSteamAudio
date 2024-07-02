#include "Engine/SoundAsset.h"
#include "AudioAllocators.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "Engine/ISoundEngine.h"
#include "IAudioInterfacesCommonData.h"
#include "SteamAudio/SteamAudioTypeIds.h"

#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{
    AZ_RTTI_NO_TYPE_INFO_IMPL(SaSoundAsset, AZ::Data::AssetData);
    AZ_TYPE_INFO_WITH_NAME_IMPL(SaSoundAsset, "SteamAudioSoundAsset", SaSoundAssetTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(SaSoundAsset, Audio::AudioImplAllocator);

    namespace Internal
    {
        struct Sound : SaSoundAsset::ISoundImpl
        {
            ma_sound m_data{};
        };
    }  // namespace Internal

    void SaSoundAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->RegisterGenericType<Buffer>();
            serialize->Class<SaSoundAsset, AZ::Data::AssetData>()->Version(0)->Field(
                "Buffer", &SaSoundAsset::m_buffer);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<SaSoundAsset>("SteamAudio Sound Asset", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute("AutoExpand", true);
            }
        }
    }

    SaSoundAsset::SaSoundAsset() = default;

    SaSoundAsset::SaSoundAsset(
        Audio::AudioInputSourceType sourceType,
        Audio::AudioInputSampleType sampleType,
        Buffer buffer)
        : m_sourceType(sourceType)
        , m_sampleType(sampleType)
        , m_buffer{ AZStd::move(buffer) }
    {
        SoundEngineNotificationBus::Handler::BusConnect();
    }

    SaSoundAsset::~SaSoundAsset()
    {
        SoundEngineNotificationBus::Handler::BusDisconnect();

        if (m_sound)
        {
            auto* sound{ static_cast<Internal::Sound*>(m_sound.get()) };
            ma_sound_uninit(&sound->m_data);
            m_sound = nullptr;
        }
    }

    void SaSoundAsset::CopySoundInto(ma_sound* copy)
    {
        ma_engine* engine{};
        SoundEngineRequestBus::BroadcastResult(engine, &ISoundEngine::GetLowLevelEngine);
        m_sound ? ma_sound_init_copy(engine, copy, MA_SOUND_FLAG_DECODE, nullptr, nullptr)
                : MA_NO_DATA_AVAILABLE;
    }
}  // namespace SteamAudio
