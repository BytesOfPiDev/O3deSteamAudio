#include "Engine/Tasks/Task.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Engine/SoundConfig.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(TaskDefinition, "EventTask", TaskDefinitionTypeId)

    void TaskDefinition::Reflect(AZ::ReflectContext* context)
    {
        SoundTaskConfig::Reflect(context);
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<TaskDefinition>()->Version(3)->Field(
                "Config", &TaskDefinition::m_config);

            if (auto* edit = serialize->GetEditContext())
            {
                edit->Class<TaskDefinition>("Task Definition", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &TaskDefinition::m_config,
                        "Configuration",
                        "")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility,
                        AZ::Edit::PropertyVisibility::ShowChildrenOnly);
            }
        }
    }

}  // namespace SteamAudio
