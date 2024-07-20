#include "Engine/AudioEvent.h"

#include "AzCore/Console/ILogger.h"

#include "Engine/Id.h"

namespace SteamAudio
{

    SaEvent::SaEvent()
        : m_startFunc(EmptyEventTask)
        , m_stopFunc(EmptyEventTask)
    {
    }

    SaEvent::SaEvent(StartFunc startFunc, StopFunc stopFunc)
        : m_startFunc{ AZStd::move(startFunc) }
        , m_stopFunc{ AZStd::move(stopFunc) } {};

    void SaEvent::Update(float)
    {
    }

    void SaEvent::Start(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Start(objectId: %llu)", objectId);
        m_startFunc ? m_startFunc(objectId) : void();
    }

    void SaEvent::Stop(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Stop(objectId: %llu)", objectId);
        m_stopFunc ? m_stopFunc(objectId) : void();
    };
}  // namespace SteamAudio
