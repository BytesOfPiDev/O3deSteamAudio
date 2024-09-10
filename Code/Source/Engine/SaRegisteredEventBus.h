#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/EBus/EBus.h"
#include "Engine/Id.h"
#include "Engine/SaEventAsset.h"
namespace SteamAudio
{
    class SaRegisteredEventBusRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaRegisteredEventBusRequests);

        SaRegisteredEventBusRequests() = default;
        virtual ~SaRegisteredEventBusRequests() = default;
        [[nodiscard]] virtual auto GetEvent() const -> AZ::Data::Asset<SaEventAsset> = 0;
    };

    struct SaRegisteredEventRequestBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SaEventId;
    };

    using SaRegisteredEventRequestBus =
        AZ::EBus<SaRegisteredEventBusRequests, SaRegisteredEventRequestBusTraits>;
}  // namespace SteamAudio
