#pragma once

#include "Engine/Id.h"

namespace SteamAudio
{
    struct StartEventData
    {
        SaEventId m_eventId;
        SaAudioObjectId m_gameObjectId;
        AZ::Name m_eventName;
    };

    struct StopEventData
    {
        SaEventId m_eventId;
        SaAudioObjectId m_gameObjectId;
        SaId m_implTriggerId;
    };
}  // namespace SteamAudio
