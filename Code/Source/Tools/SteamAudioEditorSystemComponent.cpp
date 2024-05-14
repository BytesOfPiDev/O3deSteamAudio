#include "SteamAudioEditorSystemComponent.h"

#include "AzCore/Debug/Trace.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Serialization/SerializeContext.h"

#include "AzCore/Utils/Utils.h"
#include "Engine/Configuration.h"
#include "IAudioSystemEditor.h"
#include "SteamAudio/SteamAudioTypeIds.h"
#include "Tools/AudioSystemEditor_steamaudio.h"

namespace SteamAudio
{
    AZ_COMPONENT_IMPL(
        SteamAudioEditorSystemComponent,
        "SteamAudioEditorSystemComponent",
        SteamAudioEditorSystemComponentTypeId,
        BaseSystemComponent);

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

    void SteamAudioEditorSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("SteamAudioEditorService"));
    }

    void SteamAudioEditorSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("SteamAudioEditorService"));
    }

    void SteamAudioEditorSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void SteamAudioEditorSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void SteamAudioEditorSystemComponent::Init()
    {
        SteamAudioSystemComponent::Init();

        auto const projectSrcPath = []() -> AZ::IO::FixedMaxPath
        {
            AZ::IO::FixedMaxPath path{ AZ::Utils::GetProjectPath() };
            path /= "Sounds/steamaudio_project";

            return path;
        }();

        AZ::IO::FileIOBase::GetInstance()->SetAlias(ProjectSrcAlias, projectSrcPath.c_str());

        AZ_Info(
            TYPEINFO_Name(),
            "Registering Alias: [%s: %s]\n",
            ProjectSrcAlias,
            projectSrcPath.c_str());
    }

    void SteamAudioEditorSystemComponent::Activate()
    {
        SteamAudioSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusConnect();
    }

    void SteamAudioEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        SteamAudioSystemComponent::Deactivate();
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusDisconnect();
    }

    void SteamAudioEditorSystemComponent::InitializeEditorImplPlugin()
    {
        m_editorImplPlugin = AZStd::make_unique<AudioSystemEditor_steamaudio>();
    }

    void SteamAudioEditorSystemComponent::ReleaseEditorImplPlugin()
    {
        m_editorImplPlugin.reset();
    }

    auto SteamAudioEditorSystemComponent::GetEditorImplPlugin()
        -> AudioControls::IAudioSystemEditor*
    {
        return m_editorImplPlugin.get();
    }

} // namespace SteamAudio
