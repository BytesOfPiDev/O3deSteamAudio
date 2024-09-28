#include "Engine/SoundConfig.h"

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace SteamAudio
{
    void SoundConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SoundConfig>()
                ->Version(2)
                ->Field("Volume", &SoundConfig::m_volume)
                ->Attribute(AZ::Edit::Attributes::Min, SoundConfig::VolumeMin)
                ->Attribute(AZ::Edit::Attributes::Max, SoundConfig::VolumeMax)
                ->Attribute(AZ::Edit::Attributes::Step, SoundConfig::VolumeStep)
                ->Field("Loop", &SoundConfig::m_loop);

            if (auto* edit = serialize->GetEditContext())
            {
                edit->Class<SoundConfig>("Sound Task Data", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SoundConfig::m_volume, "Volume", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SoundConfig::m_loop, "Loop", "");
            }
        }
    }

}  // namespace SteamAudio
