#include "Builder/SteamAudioAssetBuilderComponent.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

#include "Builder/AudioEventAssetBuilderWorker.h"
#include "Builder/SoundAssetBuilderWorker.h"
#include "Clients/SteamAudioSystemComponent.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/SoundAsset.h"

namespace SteamAudio
{

    void SteamAudioAssetBuilderComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SteamAudioAssetBuilderComponent, AZ::Component>()
                ->Version(0)
                ->Attribute(
                    AZ::Edit::Attributes::SystemComponentTags,
                    AZStd::vector<AZ::Crc32>({ AssetBuilderSDK::ComponentTags::AssetBuilder }));

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<SteamAudioAssetBuilderComponent>(
                    "BopAudioAssetBuilderComponent", "");
            }
        }
    }

    SteamAudioAssetBuilderComponent::SteamAudioAssetBuilderComponent() = default;
    SteamAudioAssetBuilderComponent::~SteamAudioAssetBuilderComponent() = default;

    void SteamAudioAssetBuilderComponent::Init()
    {
    }

    void SteamAudioAssetBuilderComponent::Activate()
    {
        SteamAudioSystemComponent::RegisterFileAliases();

        m_eventAssetHandler.Register();
        m_soundAssetHandler.Register();

        ConfigureAudioControlBuilder();
        ConfigureAudioEventBuilder();
        ConfigureSaSoundBuilder();
    }

    void SteamAudioAssetBuilderComponent::Deactivate()
    {
        m_audioControlBuilder.BusDisconnect();
        m_eventBuilder.BusDisconnect();
        m_saSoundBuilder.BusDisconnect();

        m_eventAssetHandler.Unregister();
        m_soundAssetHandler.Unregister();
    }

    void SteamAudioAssetBuilderComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("SteamAudioAssetBuilderService"));
    }

    void SteamAudioAssetBuilderComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("SteamAudioAssetBuilderService"));
    }

    void SteamAudioAssetBuilderComponent::GetRequiredServices(
        AZ::ComponentDescriptor::DependencyArrayType& /*required*/) {};

    void SteamAudioAssetBuilderComponent::GetDependentServices(
        AZ::ComponentDescriptor::DependencyArrayType& /*dependent*/) {};

    void SteamAudioAssetBuilderComponent::ConfigureAudioEventBuilder()
    {
        AssetBuilderSDK::AssetBuilderDesc builderDescriptor{};
        builderDescriptor.m_name = "SteamAudio Event Builder";

        builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            SaEventAsset::SourceExtensionRegex,
            AssetBuilderSDK::AssetBuilderPattern::PatternType::Regex));

        builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            SaEventAsset::ProductExtensionRegex,
            AssetBuilderSDK::AssetBuilderPattern::PatternType::Regex));

        builderDescriptor.m_busId = azrtti_typeid<AudioEventAssetBuilderWorker>();
        builderDescriptor.m_version = 0;
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

    void SteamAudioAssetBuilderComponent::ConfigureSaSoundBuilder()
    {
        AssetBuilderSDK::AssetBuilderDesc builderDescriptor{};
        builderDescriptor.m_name = "SteamAudio SaSound Builder";

        builderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(
            SaSoundAsset::RawExtensionRegex,
            AssetBuilderSDK::AssetBuilderPattern::PatternType::Regex));

        builderDescriptor.m_busId = azrtti_typeid<SaSoundAssetBuilderWorker>();
        builderDescriptor.m_version = 0;
        builderDescriptor.m_createJobFunction =
            [ObjectPtr = &m_saSoundBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->CreateJobs(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };
        builderDescriptor.m_processJobFunction =
            [ObjectPtr = &m_saSoundBuilder](auto&& PH1, auto&& PH2)
        {
            ObjectPtr->ProcessJob(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        };

        m_saSoundBuilder.BusConnect(builderDescriptor.m_busId);

        AssetBuilderSDK::AssetBuilderBus::Broadcast(
            &AssetBuilderSDK::AssetBuilderBus::Events::RegisterBuilderInformation,
            builderDescriptor);
    }

    void SteamAudioAssetBuilderComponent::ConfigureAudioControlBuilder()
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

}  // namespace SteamAudio
