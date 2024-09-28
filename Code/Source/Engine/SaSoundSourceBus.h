#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/base.h"
#include "Engine/SaSoundAsset.h"

namespace SteamAudio
{
    class SaSoundSourceRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaSoundSourceRequests);
        AZ_TYPE_INFO_WITH_NAME(
            SaSoundSourceRequests,
            "SaSoundSourceRequests",
            "{D10C5723-52B4-4030-B929-972125AC6F05}")

        SaSoundSourceRequests() = default;
        virtual ~SaSoundSourceRequests() = default;
        [[nodiscard]] virtual auto CreateInstance() const -> AZStd::any = 0;
    };

    struct SaSoundSourceRequestBusTraits : public AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Name;
    };

    using SaSoundSourceRequestBus =
        AZ::EBus<SaSoundSourceRequests, SteamAudio::SaSoundAssetBusTraits>;

}  // namespace SteamAudio
