#pragma once

#include "AzToolsFramework/ToolsComponents/EditorComponentBase.h"

namespace SteamAudio
{
    class EditorSteamAudioLevelComponent final
        : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(EditorSteamAudioLevelComponent, "988A7EE5-EFD9-4F2A-8A9F-C3A2EB32EEFE");
        AZ_DISABLE_COPY_MOVE(EditorSteamAudioLevelComponent);

        EditorSteamAudioLevelComponent();
        ~EditorSteamAudioLevelComponent() override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        static void Reflect(AZ::ReflectContext* context);
    };
} // namespace SteamAudio
