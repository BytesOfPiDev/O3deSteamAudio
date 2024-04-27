#pragma once

#include "ACETypes.h"
#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/base.h"
#include "AzCore/std/optional.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystemControl.h"

namespace SteamAudio
{
    AZ_ENUM_WITH_UNDERLYING_TYPE(
        SteamAudioControlType,
        AudioControls::TImplControlType,
        (Invalid, 0),
        (Event, AUDIO_BIT(0)),
        (Rtpc, AUDIO_BIT(1)),
        (Switch, AUDIO_BIT(2)),
        (AuxBus, AUDIO_BIT(3)),
        (SoundBank, AUDIO_BIT(4)),
        (GameState, AUDIO_BIT(5)),
        (SwitchGroup, AUDIO_BIT(6)),
        (GameStateGroup, AUDIO_BIT(7)),
        (Environment, AUDIO_BIT(8)));

    class AudioSystemControl_steamaudio final: public AudioControls::IAudioSystemControl
    {
    public:
        AZ_DEFAULT_COPY_MOVE(AudioSystemControl_steamaudio)

        AudioSystemControl_steamaudio() = default;
        AudioSystemControl_steamaudio(
            AZStd::string const& name, AudioControls::CID id, AudioControls::TImplControlType type);
        ~AudioSystemControl_steamaudio() override = default;

        [[nodiscard]] auto GetSoundFile() const -> AZStd::optional<AZStd::string>
        {
            return m_soundFilename;
        };
        
        void SetSoundFile(AZStd::string const& soundFilename)
        {
            m_soundFilename = soundFilename;
        }

    private:
        AZStd::string m_soundFilename{};
    };
} // namespace SteamAudio
