#include "Engine/AudioSource.h"

namespace SteamAudio
{
    AudioInputStreaming::AudioInputStreaming(Audio::SAudioInputConfig inputConfig)
        : m_inputConfig(AZStd::move(inputConfig))
    {
    }

    AudioInputStreaming::~AudioInputStreaming() = default;

    void AudioInputStreaming::ReadInput([[maybe_unused]] Audio::AudioStreamData const& data)
    {
    }

    void AudioInputStreaming::WriteOutput([[maybe_unused]] IPLAudioBuffer* akBuffer)
    {
    }

    [[nodiscard]] auto AudioInputStreaming::IsOk() const -> bool
    {
        return {};
    }

    [[nodiscard]] auto AudioInputStreaming::IsFormatValid() const -> bool
    {
        return {};
    }

    void AudioInputStreaming::OnActivated()
    {
    }

    void AudioInputStreaming::OnDeactivated()
    {
    }

    auto AudioInputStreaming::ReadStreamingInput([[maybe_unused]] Audio::AudioStreamData const&)
        -> AZStd::size_t
    {
        return {};
    }

    auto AudioInputStreaming::ReadStreamingMultiTrackInput(Audio::AudioStreamMultiTrackData&)
        -> AZStd::size_t
    {
        return {};
    }

} // namespace SteamAudio
