#pragma once

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/base.h"
#include "Engine/SaAudioObjectBus.h"
#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{
    class SoundInstance : protected SaAudioObjectNotificationBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundInstance);
        SoundInstance(AZ::Name soundName, bool loop = false, float volume = 1.0f);
        ~SoundInstance();

        void Start();
        void Stop();

        void SetPosition(Audio::SATLWorldPosition const& worldPosition);

    private:
        AZStd::unique_ptr<ma_sound> m_sound{};
        AZ::Name m_soundName{};
    };

}  // namespace SteamAudio
