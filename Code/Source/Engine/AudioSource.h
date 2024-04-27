#pragma once

#include "Engine/AudioSourceManager.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"
#include <phonon.h>

namespace SteamAudio
{

    class AudioInputStreaming
        : public AudioInputSource
        , public Audio::AudioStreamingRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioInputStreaming);

        AudioInputStreaming(Audio::SAudioInputConfig inputConfig);
        ~AudioInputStreaming() override;

        void ReadInput(Audio::AudioStreamData const& data) override;
        void WriteOutput(IPLAudioBuffer* akBuffer) override;

        [[nodiscard]] auto IsOk() const -> bool override;
        [[nodiscard]] auto IsFormatValid() const -> bool override;

        void OnActivated() override;
        void OnDeactivated() override;

        auto ReadStreamingInput(Audio::AudioStreamData const& data) -> AZStd::size_t override;

        auto ReadStreamingMultiTrackInput(Audio::AudioStreamMultiTrackData& data)
            -> AZStd::size_t override;

    private:
        Audio::SAudioInputConfig m_inputConfig{};
        [[maybe_unused]] IPLAudioBuffer m_inAudioBuffer{};
        [[maybe_unused]] IPLAudioBuffer m_outAudioBuffer{};
    };

} // namespace SteamAudio
