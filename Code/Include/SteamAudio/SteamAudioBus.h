
#pragma once

#include <SteamAudio/SteamAudioTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace SteamAudio
{
    class SteamAudioRequests
    {
    public:
        AZ_RTTI(SteamAudioRequests, SteamAudioRequestsTypeId);
        virtual ~SteamAudioRequests() = default;
        // Put your public methods here
    };

    class SteamAudioBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using SteamAudioRequestBus = AZ::EBus<SteamAudioRequests, SteamAudioBusTraits>;
    using SteamAudioInterface = AZ::Interface<SteamAudioRequests>;

} // namespace SteamAudio
