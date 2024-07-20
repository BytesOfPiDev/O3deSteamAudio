#pragma once

#include "AzCore/EBus/EBus.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/Id.h"
#include "Engine/Parameters.h"

extern "C" {
struct ma_engine;
struct ma_resource_manager;
}

namespace SteamAudio
{
    static constexpr auto* s_lowLevelEngineEnvName{ "ma_engine" };

    class ISoundEngine
    {
    public:
        AZ_RTTI(ISoundEngine, "{51A10EAC-C03B-4583-9594-C25CC8A4755D}");
        AZ_DISABLE_COPY_MOVE(ISoundEngine);

        ISoundEngine() = default;
        virtual ~ISoundEngine() = default;

        [[nodiscard]] virtual auto IsInitialized() const -> bool
        {
            return false;
        }

        virtual auto Initialize() -> EngineNullOutcome
        {
            return AZ::Failure("Unimplemented");
        };

        virtual auto Shutdown() -> EngineNullOutcome
        {
            return AZ::Failure("Unimplemented");
        };

        virtual auto RegisterAudioObject(SaGameObjectId const& /*objectId*/) -> EngineNullOutcome
        {
            return {};
        }

        virtual void MuteAll()
        {
        }

        virtual void UnmuteAll()
        {
        }

        virtual void GetFocus()
        {
        }

        virtual void LoseFocus()
        {
        }

        virtual void DoNothing() const
        {
        }

        virtual auto Unregister(SaGameObjectId const& /*objectId*/) -> EngineNullOutcome
        {
            return {};
        }

        [[nodiscard]] virtual auto ReportEvent(StartEventData const&) -> EngineNullOutcome
        {
            return {};
        }

        virtual auto StopEvent(SaInstanceId const&) -> bool
        {
            return {};
        }

        virtual auto CreateNewEvent() -> SaId
        {
            return {};
        }

        virtual void DestroyAudioEvent(SaId /*eventId*/)
        {
        }
    };

    class SoundEngineRequestBusTraits : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using SoundEngineRequestBus = AZ::EBus<ISoundEngine, SoundEngineRequestBusTraits>;

    class SoundEngineNotifications
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundEngineNotifications);

        SoundEngineNotifications() = default;
        virtual ~SoundEngineNotifications() = default;

        virtual void OnSoundManagerReady() const {};
        virtual void OnEventmanagerReady() const {};
    };

    struct SoundEngineNotificationBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using SoundEngineNotificationBus =
        AZ::EBus<SoundEngineNotifications, SoundEngineNotificationBusTraits>;

}  // namespace SteamAudio
