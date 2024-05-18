#include "Engine/AudioEvent.h"
#include "Engine/Id.h"

#include "miniaudio.h"

namespace SteamAudio
{
    void SaEvent::Update(float)
    {
    }

    void SaEvent::Start(SaGameObjectId)
    {
        m_startFunc();
    }

    void SaEvent::Stop(SaGameObjectId)
    {
        m_stopFunc();
    };
}  // namespace SteamAudio
