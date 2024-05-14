
#pragma once

#include "AzCore/base.h"
#include "AzToolsFramework/API/ToolsApplicationAPI.h"
#include "IAudioSystemEditor.h"

#include "Clients/SteamAudioSystemComponent.h"

namespace SteamAudio
{
    /// System component for SteamAudio editor
    class SteamAudioEditorSystemComponent
        : public SteamAudioSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
        , protected AudioControlsEditor::EditorImplPluginEventBus::Handler
    {
        using BaseSystemComponent = SteamAudioSystemComponent;

    public:
        AZ_COMPONENT_DECL(SteamAudioEditorSystemComponent);
        AZ_DISABLE_COPY_MOVE(SteamAudioEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        SteamAudioEditorSystemComponent();
        ~SteamAudioEditorSystemComponent();

    protected:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        ////////////////////////////////////////////////////////////////////////
        // AudioControlsEditor::EditorImplPluginEventBus interface
        // implementation
        void InitializeEditorImplPlugin() override;
        void ReleaseEditorImplPlugin() override;
        auto GetEditorImplPlugin() -> AudioControls::IAudioSystemEditor* override;
        ////////////////////////////////////////////////////////////////////////

    private:
        AZStd::unique_ptr<AudioControls::IAudioSystemEditor> m_editorImplPlugin;
    };
} // namespace SteamAudio
