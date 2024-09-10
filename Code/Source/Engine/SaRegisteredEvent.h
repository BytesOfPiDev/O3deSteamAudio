#pragma once

#include "AzCore/Console/ILogger.h"
#include "Engine/Id.h"
#include "Engine/SaRegisteredEventBus.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{

    class SaRegisteredEvent : protected SaRegisteredEventRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaRegisteredEvent);

        SaRegisteredEvent() = default;
        SaRegisteredEvent(SaEventId eventIdOverride)
            : m_eventId{ eventIdOverride }
        {
            AZ_Error(
                "SaRegisteredEvent",
                eventIdOverride != InvalidEventId,
                "Invalid event id override. Event bus will not connect.");
            if (eventIdOverride == InvalidEventId)
            {
                return;
            }

            SaRegisteredEventRequestBus::Handler::BusConnect(eventIdOverride);
            AZLOG_INFO("Event '%llu' registered.", static_cast<AZ::u64>(eventIdOverride));
        }

        SaRegisteredEvent(SaEventId eventIdOverride, AZ::Data::Asset<SaEventAsset> eventAsset)
            : SaRegisteredEvent(eventIdOverride)
        {
            m_asset = AZStd::move(eventAsset);
        }

        SaRegisteredEvent(AZ::Data::Asset<SaEventAsset> eventAsset)
            : m_asset{ AZStd::move(eventAsset) }
        {
            m_eventId = m_asset->GetEventId();
            if (m_eventId == InvalidEventId)
            {
                return;
            }

            auto const loadResult{ m_asset.QueueLoad() ? m_asset.BlockUntilLoadComplete()
                                                       : AZ::Data::AssetData::AssetStatus{} };

            if (loadResult != AZ::Data::AssetData::AssetStatus::Ready)
            {
                AZLOG_ERROR(
                    "Failed to register event '%llu'", static_cast<AZ::u64>(m_asset->GetEventId()));
                return;
            }
            SaRegisteredEventRequestBus::Handler::BusConnect(m_eventId);
        }

        ~SaRegisteredEvent() override
        {
            SaRegisteredEventRequestBus::Handler::BusDisconnect();
        }

        [[nodiscard]] auto GetEvent() const -> AZ::Data::Asset<SaEventAsset> override
        {
            return m_asset;
        }

    private:
        AZ::Data::Asset<SaEventAsset> m_asset{};
        SaEventId m_eventId{};
    };
}  // namespace SteamAudio
