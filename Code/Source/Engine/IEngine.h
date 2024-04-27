#pragma once

#include "Engine/Common_steamaudio.h"

namespace SteamAudio
{
    class IEngine
    {
    public:
        AZ_DISABLE_COPY_MOVE(IEngine);

        IEngine() = default;
        virtual ~IEngine() = default;

        virtual auto Initialize() -> EngineNullOutcome = 0 ;
        virtual auto Shutdown() -> EngineNullOutcome = 0;
    };
}