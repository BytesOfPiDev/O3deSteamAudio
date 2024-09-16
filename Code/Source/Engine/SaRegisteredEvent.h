#pragma once

#include "Engine/SaRegisteredEventBus.h"

namespace SteamAudio
{

    class SaRegisteredEvent : protected SaRegisteredEventRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(SaRegisteredEvent);

        SaRegisteredEvent() = default;

        SaRegisteredEvent(AZ::Data::Asset<SaEventAsset> eventAsset, bool ignoreAssetData = false);

        ~SaRegisteredEvent() override;

        [[nodiscard]] auto GetEvent() const -> AZ::Data::Asset<SaEventAsset> override
        {
            return m_asset;
        }

    private:
        AZ::Data::Asset<SaEventAsset> m_asset{};
    };
}  // namespace SteamAudio
