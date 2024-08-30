#include "Engine/Tasks/Task.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Engine/SoundConfig.h"

namespace SteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(TaskDefinition, "EventTask", "A6F92268-DA9B-4C4E-B71F-B694F18A68B5")
    AZ_TYPE_INFO_SPECIALIZE_WITH_NAME_IMPL(
        TaskType, "TaskAction", "45735319-B512-475E-BF13-4F72DC58A443");

    void TaskDefinition::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            TaskTypeReflect(*serialize);

            serialize->Class<TaskDefinition>()
                ->Version(0)
                ->Field("Config", &TaskDefinition::m_config)
                ->Field("Action", &TaskDefinition::m_taskType);

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
                        AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(
                        AZ::Edit::UIHandlers::ComboBox, &TaskDefinition::m_taskType, "Action", "");
            }
        }
    }

    TaskDefinition::TaskDefinition()
        : m_config{ SoundTaskConfig{} }
    {
    }  // FIXME: Forcing sound task for now
}  // namespace SteamAudio
