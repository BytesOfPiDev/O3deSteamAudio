#include "SteamAudioEditorSystemComponent.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioSystemEditor.h"

#include "Builder/AudioControlBuilderWorker.h"
#include "Builder/AudioEventAssetBuilderWorker.h"
#include "Builder/SoundAssetBuilderWorker.h"
#include "Engine/SaEventAsset.h"
#include "Engine/SaSoundAsset.h"
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
                ->Version(1)
                ->Attribute(
                    AZ::Edit::Attributes::SystemComponentTags,
                    AZStd::vector<AZ::Crc32>({ AssetBuilderSDK::ComponentTags::AssetBuilder }));
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
        dependent.push_back(AZ_CRC_CE("AssetDatabaseService"));
        dependent.push_back(AZ_CRC_CE("AssetCatalogService"));
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void SteamAudioEditorSystemComponent::Init()
    {
        SteamAudioSystemComponent::Init();
    }

    void SteamAudioEditorSystemComponent::Activate()
    {
        SteamAudioSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusConnect();

        ConfigureAudioEventBuilder();
        ConfigureAudioControlBuilder();
        ConfigureSaSoundBuilder();
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

    void SteamAudioEditorSystemComponent::ConfigureAudioControlBuilder()
    {
        // Register Audio Control builder
        AssetBuilderSDK::AssetBuilderDesc builderDescriptor;
        builderDescriptor.m_name = "SteamAudio Control Builder";
        // pattern finds all Audio Control xml files in the libs/gameaudio
        // folder and any of its subfolders.
        builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            R"((.*libs\/gameaudio\/).*\.xml)",
            AssetBuilderSDK::AssetBuilderPattern::PatternType::Regex));
        builderDescriptor.m_busId = azrtti_typeid<AudioControlBuilderWorker>();
        builderDescriptor.m_version = 2;
        builderDescriptor.m_createJobFunction =
            [ObjectPtr = &m_audioControlBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->CreateJobs(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
        builderDescriptor.m_processJobFunction =
            [ObjectPtr = &m_audioControlBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->ProcessJob(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };

        builderDescriptor.m_flags |=
            AssetBuilderSDK::AssetBuilderDesc::BF_DeleteLastKnownGoodProductOnFailure;

        m_audioControlBuilder.BusConnect(builderDescriptor.m_busId);

        AssetBuilderSDK::AssetBuilderBus::Broadcast(
            &AssetBuilderSDK::AssetBuilderBus::Events::RegisterBuilderInformation,
            builderDescriptor);
    }

    void SteamAudioEditorSystemComponent::ConfigureAudioEventBuilder()
    {
        AssetBuilderSDK::AssetBuilderDesc builderDescriptor{};
        builderDescriptor.m_name = "SteamAudio Event Builder";

        /*
              builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
                  SaEventAsset::SourceExtensionRegex,
                  AssetBuilderSDK::AssetBuilderPattern::PatternType::Regex));
        */

        builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            SaEventAsset::ExtensionWildcard,
            AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));

        builderDescriptor.m_busId = azrtti_typeid<AudioEventAssetBuilderWorker>();
        builderDescriptor.m_version = 2;
        builderDescriptor.m_createJobFunction =
            [ObjectPtr = &m_eventBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->CreateJobs(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
        builderDescriptor.m_processJobFunction =
            [ObjectPtr = &m_eventBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->ProcessJob(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };

        m_eventBuilder.BusConnect(builderDescriptor.m_busId);

        AssetBuilderSDK::AssetBuilderBus::Broadcast(
            &AssetBuilderSDK::AssetBuilderBus::Events::RegisterBuilderInformation,
            builderDescriptor);
    }

    void SteamAudioEditorSystemComponent::ConfigureSaSoundBuilder()
    {
        // Register MiniSound Asset
        auto* materialAsset = aznew SaSoundAssetGenericHandler(
            "SteamAudio Sound Asset", SaSoundAsset::AssetGroup, SaSoundAsset::ProductExtension);
        materialAsset->Register();
        m_assetHandlers.emplace_back(materialAsset);

        // Register MiniSound Asset Builder
        AssetBuilderSDK::AssetBuilderDesc materialAssetBuilderDescriptor;
        materialAssetBuilderDescriptor.m_name = "MiniSound Asset Builder";
        materialAssetBuilderDescriptor.m_version = 3;  // bump this to rebuild all sound files
        materialAssetBuilderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            "*.ogg", AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));
        materialAssetBuilderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            "*.flac", AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));
        materialAssetBuilderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            "*.mp3", AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));
        materialAssetBuilderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            "*.wav", AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));
        materialAssetBuilderDescriptor.m_busId = azrtti_typeid<SaSoundAssetBuilderWorker>();
        materialAssetBuilderDescriptor.m_createJobFunction =
            [this](
                AssetBuilderSDK::CreateJobsRequest const& request,
                AssetBuilderSDK::CreateJobsResponse& response)
        {
            m_soundAssetBuilder.CreateJobs(request, response);
        };
        materialAssetBuilderDescriptor.m_processJobFunction =
            [this](
                AssetBuilderSDK::ProcessJobRequest const& request,
                AssetBuilderSDK::ProcessJobResponse& response)
        {
            m_soundAssetBuilder.ProcessJob(request, response);
        };
        m_soundAssetBuilder.BusConnect(materialAssetBuilderDescriptor.m_busId);
        AssetBuilderSDK::AssetBuilderBus::Broadcast(
            &AssetBuilderSDK::AssetBuilderBus::Handler::RegisterBuilderInformation,
            materialAssetBuilderDescriptor);
    }

}  // namespace SteamAudio
