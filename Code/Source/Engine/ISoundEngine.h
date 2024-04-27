#pragma once

#include "AzCore/Component/ComponentApplicationBus.h"
#include "Engine/AudioObject.h"
#include "Engine/Common_steamaudio.h"

namespace SteamAudio
{

    struct StartEventData
    {
        SaGameObjectId m_gameObjectId;
    };

    class ISoundEngine
    {
    public:
        AZ_TYPE_INFO(ISoundEngine, "{51A10EAC-C03B-4583-9594-C25CC8A4755D}");
        AZ_DISABLE_COPY_MOVE(ISoundEngine);

        ISoundEngine() = default;
        virtual ~ISoundEngine() = default;

        virtual auto Initialize() -> EngineNullOutcome = 0;
        virtual auto Shutdown() -> EngineNullOutcome = 0;
        virtual auto RegisterAudioObject(SaGameObjectId const& objectId) -> EngineNullOutcome
        {

            return {};
        }

        virtual auto Unregister(SaGameObjectId const& /*objectId*/) -> EngineNullOutcome
        {
            return {};
        }

        virtual auto StartEvent(StartEventData const&) -> SaInstanceId
        {
            return {};
        };
    };
} // namespace SteamAudio
