#include "Engine/SoundAsset.h"

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

#include "Engine/ISoundEngine.h"
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
            serialize->Class<SaSoundAsset, AZ::Data::AssetData>()->Version(1)->Field(
                "Buffer", &SaSoundAsset::m_data);

            serialize->RegisterGenericType<AZ::Data::Asset<SaSoundAsset>>();

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

    SaSoundAsset::~SaSoundAsset()
    {
        SoundEngineNotificationBus::Handler::BusDisconnect();
    }

}  // namespace SteamAudio
