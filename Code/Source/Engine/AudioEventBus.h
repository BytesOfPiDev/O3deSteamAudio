#pragma once

#include "AzCore/EBus/EBus.h"
#include "Engine/Id.h"
namespace SteamAudio
{
    class AudioEventRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioEventRequests);

        AudioEventRequests() = default;
        virtual ~AudioEventRequests() = default;

        virtual void StartEventById(SaEventId event) = 0;
        virtual void StopEventById(SaEventId event) = 0;
    };

    struct AudioEventRequestBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SaGameObjectId;
    };

    using SaEventRequestBus = AZ::EBus<AudioEventRequests, AudioEventRequestBusTraits>;

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
