#include "Engine/AudioEvent.h"
#include "Engine/Id.h"

#include "AzCore/Console/ILogger.h"

namespace SteamAudio
{

    SaEvent::SaEvent(AZ::Data::AssetId)
    {
    }

    void SaEvent::Update(float)
    {
    }

    void SaEvent::Start(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Start(objectId: %llu)", objectId);
        m_startFunc();
    }

    void SaEvent::Stop(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Stop(objectId: %llu)", objectId);
        m_stopFunc();
    };
}  // namespace SteamAudio
