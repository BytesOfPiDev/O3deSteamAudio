#pragma once

#include "Engine/IEngine.h"

namespace SteamAudio
{
    class SteamAudioEngine : public IEngine
    {
    public:
        auto Initialize() -> EngineNullOutcome override
        {
            return AZ::Failure("Unimplemented");
        }

        auto Shutdown() -> EngineNullOutcome override
        {
            return AZ::Failure("Unimplemented");
        }
    };
} // namespace SteamAudio