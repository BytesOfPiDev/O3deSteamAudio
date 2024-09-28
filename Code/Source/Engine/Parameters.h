#pragma once

#include "Engine/Id.h"

namespace SteamAudio
{
    struct StartEventData
    {
        [[nodiscard]] auto IsValid() const -> bool
        {
            return (m_eventId.IsValid()) && (m_gameObjectId != InvalidSaGameObjectId);
        }

        SaEventId m_eventId;
        SaAudioObjectId m_gameObjectId;
    };

    struct StopEventData
    {
        SaEventId m_eventId;
        SaAudioObjectId m_gameObjectId;
        SaId m_implTriggerId;
    };
}  // namespace SteamAudio
