#include "SteamAudioSystemComponent.h"

#include "AzCore/Console/ILogger.h"
#include "AzCore/PlatformId/PlatformDefaults.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/Settings/SettingsRegistry.h"

#include "IAudioSystem.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_COMPONENT_IMPL(
        SteamAudioSystemComponent, "SteamAudioSystemComponent", SteamAudioSystemComponentTypeId);

    void SteamAudioSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SteamAudioSystemComponent, AZ::Component>()->Version(0);
        }
    }

    void SteamAudioSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("AudioEngineService"));
        provided.push_back(AZ_CRC_CE("SteamAudioService"));
    }

    void SteamAudioSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("AudioEngineService"));
        incompatible.push_back(AZ_CRC_CE("SteamAudioService"));
    }

    void SteamAudioSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("AssetCatalogService"));
        required.push_back(AZ_CRC_CE("AssetDatabaseService"));
    }

    void SteamAudioSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("AudioSystemService"));
    }

    SteamAudioSystemComponent::SteamAudioSystemComponent()
    {
        if (SteamAudioInterface::Get() == nullptr)
        {
            SteamAudioInterface::Register(this);
        }

        Audio::Gem::EngineRequestBus::Handler::BusConnect();
    }

    SteamAudioSystemComponent::~SteamAudioSystemComponent()
    {
        if (SteamAudioInterface::Get() == this)
        {
            SteamAudioInterface::Unregister(this);
        }

        Audio::Gem::EngineRequestBus::Handler::BusDisconnect();
    }

    void SteamAudioSystemComponent::Init()
    {
    }

    void SteamAudioSystemComponent::Activate()
    {
        SteamAudioRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void SteamAudioSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        SteamAudioRequestBus::Handler::BusDisconnect();
    }

    void SteamAudioSystemComponent::OnTick(
        [[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

    auto SteamAudioSystemComponent::Initialize() -> bool
    {
        AZ_Verify(
            m_steamAudioEngine.Initialize().IsSuccess(), "Failed to initialize MiniAudio engine!");

        Audio::SystemRequest::Initialize initRequest;
        AZ::Interface<Audio::IAudioSystem>::Get()->PushRequestBlocking(AZStd::move(initRequest));

        return true;
    }

    void SteamAudioSystemComponent::Release()
    {
    }

} // namespace SteamAudio
