#include "Engine/Sound.h"

#include "Engine/SoundAsset.h"
#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{
    Sound::Sound(AZ::Data::Asset<SaSoundAsset> soundAsset)
        : m_soundAsset{ AZStd::move(soundAsset) }
    {
        m_lowLevelSound = AZStd::make_any<ma_sound>();
    }
}  // namespace SteamAudio
