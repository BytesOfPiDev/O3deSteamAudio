#include "Engine/SoundAsset.h"

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/ISoundEngine.h"
#include "Engine/ResourceManager.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/SteamAudioTypeIds.h"

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
            serialize->RegisterGenericType<BufferView>();
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

    SaSoundAsset::SaSoundAsset()
    {
        SoundEngineNotificationBus::Handler::BusConnect();
    };

    SaSoundAsset::SaSoundAsset(
        Audio::AudioInputSourceType sourceType,
        Audio::AudioInputSampleType sampleType,
        AZ::u32 channelCount,
        AZ::u32 sampleRate,
        Buffer buffer)
        : SteamAudio::SaSoundAsset()
    {
        SetAudioData(sourceType, sampleType, channelCount, sampleRate, AZStd::move(buffer));
    }

    SaSoundAsset::SaSoundAsset(
        Audio::AudioInputSourceType sourceType,
        Audio::AudioInputSampleType sampleType,
        AZ::u32 channelCount,
        AZ::u32 sampleRate,
        BufferView buffer)
        : SteamAudio::SaSoundAsset()
    {
        SetAudioData(
            sourceType, sampleType, channelCount, sampleRate, Buffer(buffer.begin(), buffer.end()));
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

    void SaSoundAsset::SetAudioData(Audio::AudioInputSourceType sourceType, Buffer buffer)
    {
        m_sourceType = sourceType;
        m_sampleType = Audio::AudioInputSampleType::Unsupported;
        m_buffer.swap(buffer);
        m_sampleRate = 0;
        m_channels = 0;
    }

    void SaSoundAsset::SetAudioData(Audio::AudioInputSourceType sourceType, BufferView buffer)
    {
        m_sourceType = sourceType;
        m_sampleType = Audio::AudioInputSampleType::Unsupported;
        m_buffer = { buffer.begin(), buffer.end() };
        m_sampleRate = 0;
        m_channels = 0;
    }

    void SaSoundAsset::SetAudioData(
        Audio::AudioInputSourceType sourceType,
        Audio::AudioInputSampleType sampleType,
        AZ::u32 channelCount,
        AZ::u32 sampleRate,
        Buffer buffer)
    {
        m_sourceType = sourceType;
        m_sampleType = sampleType;
        m_buffer.swap(buffer);
        m_sampleRate = sampleRate;
        m_channels = channelCount;
    }

    void SaSoundAsset::SetAudioData(
        Audio::AudioInputSourceType sourceType,
        Audio::AudioInputSampleType sampleType,
        AZ::u32 channelCount,
        AZ::u32 sampleRate,
        BufferView buffer)
    {
        m_sourceType = sourceType;
        m_sampleType = sampleType;
        m_buffer = { buffer.begin(), buffer.end() };
        m_sampleRate = sampleRate;
        m_channels = channelCount;
    }
}  // namespace SteamAudio
