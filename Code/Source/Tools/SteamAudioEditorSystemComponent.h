
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/SteamAudioSystemComponent.h>

namespace SteamAudio
{
    /// System component for SteamAudio editor
    class SteamAudioEditorSystemComponent
        : public SteamAudioSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = SteamAudioSystemComponent;
    public:
        AZ_COMPONENT_DECL(SteamAudioEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        SteamAudioEditorSystemComponent();
        ~SteamAudioEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace SteamAudio
