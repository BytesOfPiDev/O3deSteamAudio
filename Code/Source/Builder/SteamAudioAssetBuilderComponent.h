#pragma once

#include "AzCore/Component/Component.h"

#include "Builder/AudioControlBuilderWorker.h"
#include "Builder/AudioEventAssetBuilderWorker.h"
#include "Builder/SoundAssetBuilderWorker.h"
#include "Engine/AudioEventAssetHandler.h"
#include "Engine/SoundAssetHandler.h"

namespace SteamAudio
{

    class SteamAudioAssetBuilderComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(SteamAudioAssetBuilderComponent, "{3FDD085D-1B0C-44E1-AB10-61C9BAD190F5}");
        AZ_DISABLE_COPY_MOVE(SteamAudioAssetBuilderComponent);

        static void Reflect(AZ::ReflectContext* context);

        SteamAudioAssetBuilderComponent();
        ~SteamAudioAssetBuilderComponent() override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& /*required*/);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void CreateFactories();

        void ConfigureSoundBankBuilder();
        void ConfigureAudioEventBuilder();
        void ConfigureAudioControlBuilder();
        void ConfigureSaSoundBuilder();

    private:
        AudioEventAssetBuilderWorker m_eventBuilder;
        AudioControlBuilderWorker m_audioControlBuilder;
        SaSoundAssetBuilderWorker m_saSoundBuilder;
        SaSoundAssetHandler m_soundAssetHandler{};
        SaEventAssetHandler m_eventAssetHandler{};
    };
}  // namespace SteamAudio
