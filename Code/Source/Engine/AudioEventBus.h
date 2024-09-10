#pragma once

#include "AzCore/EBus/EBus.h"

#include "Engine/Id.h"

namespace SteamAudio
{
    class SaEventInstanceRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaEventInstanceRequests);

        SaEventInstanceRequests() = default;
        virtual ~SaEventInstanceRequests() = default;

        virtual void StopEvent() = 0;
        virtual auto GetParentAudioObject() -> SaGameObjectId = 0;
    };

    struct SaEventInstanceRequestBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SaEventInstanceId;
    };

    using SaEventInstanceRequestBus =
        AZ::EBus<SaEventInstanceRequests, SaEventInstanceRequestBusTraits>;

    class AudioEventNotifications
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioEventNotifications);

        AudioEventNotifications() = default;
        virtual ~AudioEventNotifications() = default;

        virtual void OnStart(SaGameObjectId) = 0;
        virtual void OnStop(SaGameObjectId) = 0;
    };

    struct AudioEventNotificationBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Crc32;
    };

    using AudioEventNotificationBus =
        AZ::EBus<AudioEventNotifications, AudioEventNotificationBusTraits>;
}  // namespace SteamAudio
