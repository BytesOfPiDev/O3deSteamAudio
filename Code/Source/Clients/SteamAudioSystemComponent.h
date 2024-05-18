
#pragma once

#include "AzCore/Component/Component.h"
#include "AzCore/Component/TickBus.h"
#include "Engine/AudioEventAssetHandler.h"
#include "Engine/SoundAssetHandler.h"
#include "IAudioSystem.h"

#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/SoundEngine.h"
#include "SteamAudio/SteamAudioBus.h"

namespace SteamAudio
{
    class SteamAudioSystemComponent
        : public AZ::Component
        , protected SteamAudioRequestBus::Handler
        , public AZ::TickBus::Handler
        , protected Audio::Gem::EngineRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(SteamAudioSystemComponent);
        AZ_DISABLE_COPY_MOVE(SteamAudioSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        SteamAudioSystemComponent();
        ~SteamAudioSystemComponent() override;

        static void RegisterFileAliases();

        ////////////////////////////////////////////////////////////////////////
        // Audio::Gem::EngineRequestBus interface implementation
        auto Initialize() -> bool override;
        void Release() override;
        ////////////////////////////////////////////////////////////////////////

    protected:
        ////////////////////////////////////////////////////////////////////////
        // SteamAudioRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        //////////////////////////////////////////:w
        //////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////

    private:
        SaSoundAssetHandler m_soundAssetHandler{};
        SaEventAssetHandler m_eventAssetHandler{};
        AZStd::optional<AudioSystemImpl_steamaudio> m_audioSystemImpl{ AZStd::nullopt };
        AZStd::optional<SteamAudioEngine> m_soundEngine{ AZStd::nullopt };
    };

}  // namespace SteamAudio
