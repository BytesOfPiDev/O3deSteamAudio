#pragma once

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/base.h"
#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{
    class SoundInstance
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundInstance);
        SoundInstance(AZ::Name soundName, bool loop = false, float volume = 1.0f);
        ~SoundInstance();

        void Start();

        void Stop();

    private:
        AZStd::unique_ptr<ma_sound> m_sound{};
        AZ::Name m_soundName{};
    };

}  // namespace SteamAudio
