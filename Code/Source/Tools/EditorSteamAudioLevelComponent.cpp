#include "Tools/EditorSteamAudioLevelComponent.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace SteamAudio
{
    void EditorSteamAudioLevelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize
                ->Class<
                    EditorSteamAudioLevelComponent,
                    AzToolsFramework::Components::EditorComponentBase>()
                ->Version(0);

            if (AZ::EditContext* edit = serialize->GetEditContext())
            {
                edit->Class<EditorSteamAudioLevelComponent>("Steam Audio", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute("AutoExpand", true)
                    ->Attribute(
                        AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Level"));
            }
        }
    }

    EditorSteamAudioLevelComponent::EditorSteamAudioLevelComponent() = default;
    EditorSteamAudioLevelComponent::~EditorSteamAudioLevelComponent() = default;

    void EditorSteamAudioLevelComponent::Init()
    {
    }

    void EditorSteamAudioLevelComponent::Activate()
    {
    }

    void EditorSteamAudioLevelComponent::Deactivate()
    {
    }

} // namespace SteamAudio
