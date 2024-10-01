#include "Engine/SaSoundInstance.h"

#include "AzCore/Console/ILogger.h"

#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    SoundInstance::SoundInstance(AZ::Name soundName, bool loop, float volume)
        : m_soundName{ AZStd::move(soundName) }
        , m_sound{ aznew ma_sound }
    {
        if (!Util::GetMaEngine())
        {
            AZLOG_ERROR("SoundInstance setup failed - no ma_engine available");
            return;
        }

        auto const initResult{ ma_sound_init_from_file(
            Util::GetMaEngine(),
            m_soundName.GetCStr(),
            MA_SOUND_FLAG_DECODE,
            nullptr,
            nullptr,
            m_sound.get()) };

        if (initResult != MA_SUCCESS)
        {
            AZLOG_ERROR(
                "SoundInstance setup failed - sound init with '%s' unsuccessful: '%d'",
                m_soundName.GetCStr(),
                initResult);
            return;
        }

        ma_sound_set_volume(m_sound.get(), volume);
        ma_sound_set_looping(m_sound.get(), loop);
    }

    SoundInstance::~SoundInstance()
    {
        ma_sound_uninit(m_sound.get());
    }

    void SoundInstance::Start()
    {
        ma_sound_start(m_sound.get());
    }
    void SoundInstance::Stop()
    {
        ma_sound_stop(m_sound.get());
    }
    void SoundInstance::SetPosition(Audio::SATLWorldPosition const& worldPosition)
    {
        auto const pos{ worldPosition.GetPositionVec() };
        ma_sound_set_position(m_sound.get(), pos.GetX(), pos.GetY(), pos.GetZ());
    }
}  // namespace SteamAudio
