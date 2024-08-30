#pragma once

#include "Engine/Sound.h"
namespace SteamAudio
{
    class SoundSourceLoader
    {
    public:
        void Load();

    private:
        AZStd::vector<SoundSource> m_sources{};
    };
}  // namespace SteamAudio
