#include "Engine/SaEventAsset.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioSystem.h"

#include "Engine/Id.h"

namespace SteamAudio
{
    void SaEventAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            SaEventId::Reflect(context);
            SaEventInstanceId::Reflect(context);
            SaSoundAsset::Reflect(context);

            serialize->Class<SaEventAsset, AZ::Data::AssetData>()
                ->Version(SaEventAsset::CurrentVersion)
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Field("Name", &SaEventAsset::m_name)
                ->Field("EventId", &SaEventAsset::m_eventId)
                ->Field("Sounds", &SaEventAsset::m_soundSources);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<SaEventAsset>("SteamAudio Event Asset", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SaEventAsset::m_name, "Name", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &SaEventAsset::UpdateId)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &SaEventAsset::m_soundSources,
                        "Sounds to play",
                        "");
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
        UpdateId();
    }

    void SaEventAsset::UpdateId()
    {
        m_eventId = Audio::AudioStringToID<SaEventId>(m_name.c_str());
    }
}  // namespace SteamAudio
