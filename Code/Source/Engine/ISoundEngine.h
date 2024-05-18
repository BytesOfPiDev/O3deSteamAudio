#pragma once

#include "AzCore/EBus/EBus.h"
#include "Engine/AudioEvent.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/Id.h"
#include "Engine/Parameters.h"

namespace SteamAudio
{

    class ISoundEngine
    {
    public:
        AZ_RTTI(ISoundEngine, "{51A10EAC-C03B-4583-9594-C25CC8A4755D}");
        AZ_DISABLE_COPY_MOVE(ISoundEngine);

        ISoundEngine() = default;
        virtual ~ISoundEngine() = default;

        virtual auto Initialize() -> EngineNullOutcome = 0;
        virtual auto Shutdown() -> EngineNullOutcome = 0;
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

        // BUG: For whatever stupid reason, unique pointers can't work properly with ebuses at all,
        // so don't call this from a bus (compile-time). Or try to call it and find out.
        virtual void AddEvent(
            SaEventId /*eventId*/, AZStd::unique_ptr<SteamAudio::SaEvent> /*event*/)
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
}  // namespace SteamAudio
