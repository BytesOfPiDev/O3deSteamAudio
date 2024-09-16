#include "SaRegisteredEvent.h"

#include "AzCore/Console/ILogger.h"
#include "Engine/SaRegisteredEventBus.h"

namespace SteamAudio
{
    SaRegisteredEvent::SaRegisteredEvent(
        AZ::Data::Asset<SaEventAsset> eventAsset, bool ignoreAssetData)
        : m_asset{ AZStd::move(eventAsset) }
    {
        AZ_Error(AZ_FUNCTION_SIGNATURE, m_asset, "Asset is null - unable to register");

        if (!ignoreAssetData)
        {
            AZ_Error(
                "SaRegisteredEvent",
                !m_asset.IsError(),
                "Unable to register '%s' because it is in an error state",
                m_asset.GetHint().c_str());
        }

        SaRegisteredEventRequestBus::Handler::BusConnect(m_asset->GetEventId());
        AZLOG_INFO("Event  registered: '%llu'", m_asset->GetEventId().GetValue());
    }
    SaRegisteredEvent::~SaRegisteredEvent()
    {
        if (SaRegisteredEventRequestBus::Handler::BusIsConnected())
        {
            return;
        }
        AZLOG_INFO("Event unregistered: '%llu'", m_asset->GetEventId().GetValue());
        SaRegisteredEventRequestBus::Handler::BusDisconnect();
    }
}  // namespace SteamAudio
