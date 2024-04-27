#include "SteamAudioSystemComponent.h"

#include <AzCore/Console/Console.h>

#include "AzCore/Console/IConsoleTypes.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioSystem.h"

#include "Engine/Configuration.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_COMPONENT_IMPL(
        SteamAudioSystemComponent, "SteamAudioSystemComponent", SteamAudioSystemComponentTypeId);

    AZ_CVAR(
        bool,
        cl_enable_steamaudio_soundengine,
        true,
        nullptr,
        AZ::ConsoleFunctorFlags::ReadOnly,
        "Enable the SteamAudio AudioSystemImplementation. Takes effect during engine startup "
        "only.");

    void SteamAudioSystemComponent::RegisterFileAliases()
    {
        auto const banksPath = []()
        {
            static constexpr auto path{ "@products@/sounds/steamaudio/banks" };
            return AZ::IO::FileIOBase::GetInstance()->ResolvePath(path).value_or("");
        }();

        auto const eventsPath = []()
        {
            static constexpr auto path{ "@products@/sounds/steamaudio/events" };
            return AZ::IO::FileIOBase::GetInstance()->ResolvePath(path).value_or("");
        }();

        auto const projectPath = []()
        {
            static constexpr auto path{ "@products@/sounds/steamaudio" };
            return AZ::IO::FileIOBase::GetInstance()->ResolvePath(path).value_or("");
        }();

        AZ::IO::FileIOBase::GetInstance()->SetAlias(BanksAlias, banksPath.c_str());
        AZ::IO::FileIOBase::GetInstance()->SetAlias(EventsAlias, eventsPath.c_str());
        AZ::IO::FileIOBase::GetInstance()->SetAlias(ProjectAlias, projectPath.c_str());
        {
        }
    }

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
    }

    SteamAudioSystemComponent::~SteamAudioSystemComponent()
    {
        if (SteamAudioInterface::Get() == this)
        {
            SteamAudioInterface::Unregister(this);
        }
    }

    void SteamAudioSystemComponent::Init()
    {
        if (cl_enable_steamaudio_soundengine)
        {
            Audio::Gem::EngineRequestBus::Handler::BusConnect();
        }
        else
        {
            AZ_Warning(
                "SteamAudioSystemComponent",
                false,
                "SteamAudio AudioSystemImplementation disabled via CVAR.");
        }
    }

    void SteamAudioSystemComponent::Activate()
    {
        SteamAudioRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void SteamAudioSystemComponent::Deactivate()
    {
        if (m_audioSystemImpl.has_value())
        {
            Audio::SystemRequest::Shutdown shutdownRequest;
            AZ::Interface<Audio::IAudioSystem>::Get()->PushRequestBlocking(
                AZStd::move(shutdownRequest));
        }

        Audio::Gem::EngineRequestBus::Handler::BusDisconnect();

        AZ::TickBus::Handler::BusDisconnect();
        SteamAudioRequestBus::Handler::BusDisconnect();
    }

    void SteamAudioSystemComponent::OnTick(
        [[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

    auto SteamAudioSystemComponent::Initialize() -> bool
    {
        if (m_audioSystemImpl.has_value())
        {
            AZ_Error(
                "SteamAudioSystemComponent",
                false,
                "SteamAudio AudioSystemImplementation already initialized.");

            return false;
        }

        m_soundEngine.emplace();
        m_audioSystemImpl.emplace();

        Audio::SystemRequest::Initialize initRequest;
        AZ::Interface<Audio::IAudioSystem>::Get()->PushRequestBlocking(AZStd::move(initRequest));

        return true;
    }

    void SteamAudioSystemComponent::Release()
    {
        m_audioSystemImpl.reset();
        m_soundEngine.reset();
    }
} // namespace SteamAudio
