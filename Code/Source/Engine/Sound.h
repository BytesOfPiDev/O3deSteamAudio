#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/base.h"
#include "AzCore/std/any.h"

#include "Engine/SoundAsset.h"

extern "C" {
struct ma_sound;
}

namespace SteamAudio
{
    class Sound
    {
    public:
        AZ_DISABLE_COPY_MOVE(Sound);

        Sound(AZ::Data::Asset<SaSoundAsset> soundAsset);
        ~Sound();

        [[nodiscard]] auto GetNative() const -> ma_sound*;

    private:
        AZStd::any m_lowLevelSound{};

        AZ::Data::Asset<SaSoundAsset> m_soundAsset{};
    };

}  // namespace SteamAudio
