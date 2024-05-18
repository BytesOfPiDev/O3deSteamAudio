#include "Engine/AudioEventAsset.h"

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/AudioEventAsset.h"
#include "IAudioSystem.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    using Audio::TAudioEventID;

    AZ_RTTI_NO_TYPE_INFO_IMPL(SaEventAsset, AZ::Data::AssetData);
    AZ_TYPE_INFO_WITH_NAME_IMPL(SaEventAsset, "SteamAudio Event Asset", SaEventAssetTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(SaEventAsset, Audio::AudioImplAllocator);

    void SaEventAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SaEventAsset, AZ::Data::AssetData>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Field("Name", &SaEventAsset::m_name)
                ->Field("EventId", &SaEventAsset::m_id)
                ->Field("Sound", &SaEventAsset::m_sound);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<SaEventAsset>(TYPEINFO_Name(), "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute("AutoExpand", true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SaEventAsset::m_name, "Name", "")
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

    SaEventAsset::~SaEventAsset() = default;

    void SaEventAsset::SetEventName(AudioEventName eventName)
    {
        m_name = AZStd::move(eventName);
        eventName = "";
        m_id = Audio::AudioStringToID<SaEventId>(m_name.c_str());
    }

}  // namespace SteamAudio
