#include "Engine/SrcSaEventAsset.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(EditorSaEventAsset, AZ::SystemAllocator);
    AZ_RTTI_NO_TYPE_INFO_IMPL(EditorSaEventAsset, AZ::Data::AssetData);
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        EditorSaEventAsset, "EditorSaEvent", EditorSaEventAssetHandlerTypeId)

    void EditorSaEventAsset::SetEventName(AZStd::string eventName)
    {
        m_eventName = AZStd::move(eventName);
    };

    void EditorSaEventAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* const serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<EditorSaEventAsset, AZ::Data::AssetData>()
                ->Version(3)
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Field("Name", &EditorSaEventAsset::m_eventName)
                ->Field("SoundToPlay", &EditorSaEventAsset::m_soundAsset);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<EditorSaEventAsset>("SteamAudio Event Asset Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute("AutoExpand", true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorSaEventAsset::m_eventName, "Name", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &EditorSaEventAsset::m_soundAsset,
                        "Sound To Play",
                        "");
            }
        }
    }

}  // namespace SteamAudio
