#pragma once

#include "Engine/Common_steamaudio.h"

namespace SteamAudio
{
    class IEngine
    {
    public:
        AZ_TYPE_INFO(IEngine, "{51A10EAC-C03B-4583-9594-C25CC8A4755D}");
        AZ_DISABLE_COPY_MOVE(IEngine);

        IEngine() = default;
        virtual ~IEngine() = default;

        virtual auto Initialize() -> EngineNullOutcome = 0;
        virtual auto Shutdown() -> EngineNullOutcome = 0;
    };
} // namespace SteamAudio
