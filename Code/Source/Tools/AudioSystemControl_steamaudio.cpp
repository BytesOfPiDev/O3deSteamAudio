#include "AudioSystemControl_steamaudio.h"

#include "ACETypes.h"
#include "IAudioSystemControl.h"

namespace SteamAudio
{
    AudioSystemControl_steamaudio::AudioSystemControl_steamaudio(
        AZStd::string const& name, AudioControls::CID id, AudioControls::TImplControlType type)
        : IAudioSystemControl(name, id, type){};
} // namespace SteamAudio
