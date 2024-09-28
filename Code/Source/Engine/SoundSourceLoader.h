#pragma once

#include "Engine/SaSoundSource.h"

namespace SteamAudio
{
    class SoundSourceLoader
    {
    public:
        void Load();

    private:
        AZStd::vector<AZStd::unique_ptr<SoundSource>> m_sources{};
    };
}  // namespace SteamAudio
