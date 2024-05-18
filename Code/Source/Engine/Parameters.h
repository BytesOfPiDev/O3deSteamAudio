#pragma once

#include "Engine/Id.h"

namespace SteamAudio
{
    struct StartEventData
    {
        SaEventId m_eventId;
        SaGameObjectId m_gameObjectId;
        SaId m_implTriggerId;
    };

    struct StopEventData
    {
        SaEventId m_eventId;
        SaGameObjectId m_gameObjectId;
        SaId m_implTriggerId;
    };
}  // namespace SteamAudio
