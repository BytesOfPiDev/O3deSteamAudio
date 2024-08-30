#include "Engine/SoundConfig.h"

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Engine/SaEventAsset.h"

namespace SteamAudio
{
    void SoundTaskConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SoundTaskConfig>()
                ->Version(2)
                ->Field("Asset", &SoundTaskConfig::m_asset)
                ->Field("Volume", &SoundTaskConfig::m_volume)
                ->Attribute(AZ::Edit::Attributes::Min, SoundTaskConfig::VolumeMin)
                ->Attribute(AZ::Edit::Attributes::Max, SoundTaskConfig::VolumeMax)
                ->Attribute(AZ::Edit::Attributes::Step, SoundTaskConfig::VolumeStep)
                ->Field("Loop", &SoundTaskConfig::m_loop);

            if (auto* edit = serialize->GetEditContext())
            {
                edit->Class<SoundTaskConfig>("Sound Task Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SoundTaskConfig::m_asset, "Sound Asset", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SoundTaskConfig::m_volume, "Volume", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SoundTaskConfig::m_loop, "Loop", "");
            }
        }
    }
}  // namespace SteamAudio
