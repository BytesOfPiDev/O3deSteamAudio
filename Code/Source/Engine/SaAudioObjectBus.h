#pragma once

#include "AzCore/EBus/EBus.h"

#include "Engine/Id.h"

namespace SteamAudio
{
    class SaAudioObjectRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaAudioObjectRequests);

        SaAudioObjectRequests() = default;
        virtual ~SaAudioObjectRequests() = default;

        virtual auto PushEvent(SaEventId eventId) -> SaEventInstanceId = 0;
        virtual void PopEvent(SaEventId eventId) = 0;
        virtual void PopEvent(SaEventInstanceId instanceId) = 0;
    };

    struct SaAudioObjectRequestBusTraits : public AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SaGameObjectId;
    };

    using SaAudioObjectRequestBus = AZ::EBus<SaAudioObjectRequests, SaAudioObjectRequestBusTraits>;

    class SaAudioObjectNotifications
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaAudioObjectNotifications);

        SaAudioObjectNotifications() = default;
        virtual ~SaAudioObjectNotifications() = default;

        virtual void OnUpdate(float dt) = 0;
        virtual void OnStop() = 0;
    };

    struct SaAudioObjectNotificationBusTraits : public AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SaGameObjectId;
    };

    using SaAudioObjectRequestBus = AZ::EBus<SaAudioObjectRequests, SaAudioObjectRequestBusTraits>;
}  // namespace SteamAudio
