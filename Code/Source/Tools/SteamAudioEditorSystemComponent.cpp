
#include <AzCore/Serialization/SerializeContext.h>
#include "SteamAudioEditorSystemComponent.h"

#include <SteamAudio/SteamAudioTypeIds.h>

namespace SteamAudio
{
    AZ_COMPONENT_IMPL(SteamAudioEditorSystemComponent, "SteamAudioEditorSystemComponent",
        SteamAudioEditorSystemComponentTypeId, BaseSystemComponent);

    void SteamAudioEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SteamAudioEditorSystemComponent, SteamAudioSystemComponent>()
                ->Version(0);
        }
    }

    SteamAudioEditorSystemComponent::SteamAudioEditorSystemComponent() = default;

    SteamAudioEditorSystemComponent::~SteamAudioEditorSystemComponent() = default;

    void SteamAudioEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("SteamAudioEditorService"));
    }

    void SteamAudioEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("SteamAudioEditorService"));
    }

    void SteamAudioEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void SteamAudioEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void SteamAudioEditorSystemComponent::Activate()
    {
        SteamAudioSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void SteamAudioEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        SteamAudioSystemComponent::Deactivate();
    }

} // namespace SteamAudio
