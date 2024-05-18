#include "Engine/SoundAsset.h"
#include "AudioAllocators.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioInterfacesCommonData.h"
#include "SteamAudio/SteamAudioTypeIds.h"
namespace SteamAudio
{
    AZ_RTTI_NO_TYPE_INFO_IMPL(SaSoundAsset, AZ::Data::AssetData);
    AZ_TYPE_INFO_WITH_NAME_IMPL(SaSoundAsset, "SteamAudioSoundAsset", SaSoundAssetTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(SaSoundAsset, Audio::AudioImplAllocator);

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
    }

    SaSoundAsset::~SaSoundAsset() = default;
}  // namespace SteamAudio
