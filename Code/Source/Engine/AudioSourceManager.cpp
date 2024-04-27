#include "Engine/AudioSourceManager.h"
#include "Engine/AudioInput/AudioInputFile.h"
#include "Engine/AudioSource.h"

namespace SteamAudio
{
    auto AudioInputSource::IsFormatValid() const -> bool
    {
        // Audio Input Source has restrictions on the formats that are supported:
        // 16-bit Integer samples, interleaved samples
        // 32-bit Float samples, non-interleaved samples
        // The Parser doesn't care about such restrictions and is only responsible for
        // reading the header information and validating it.

        bool valid = true;

        if (m_config.m_sampleType == Audio::AudioInputSampleType::Int &&
            m_config.m_bitsPerSample != 16)
        {
            valid = false;
        }

        if (m_config.m_sampleType == Audio::AudioInputSampleType::Float &&
            m_config.m_bitsPerSample != 32)
        {
            valid = false;
        }

        if (m_config.m_sampleType == Audio::AudioInputSampleType::Unsupported)
        {
            valid = false;
        }

        if (!valid)
        {
            AZ_TracePrintf(
                "AudioInputFile",
                "The file format is NOT supported!  Only 16-bit integer or 32-bit float sample "
                "types are allowed!\n"
                "Current Format: (%s / %d)\n",
                m_config.m_sampleType == Audio::AudioInputSampleType::Int
                    ? "Int"
                    : (m_config.m_sampleType == Audio::AudioInputSampleType::Float ? "Float"
                                                                                   : "Unknown"),
                m_config.m_bitsPerSample);
        }

        return valid;
    }

    void AudioInputSource::SetSourceId(Audio::TAudioSourceId sourceId)
    {
        m_config.m_sourceId = sourceId;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioInputSource::GetSourceId() const -> Audio::TAudioSourceId
    {
        return m_config.m_sourceId;
    }

    AudioSourceManager::AudioSourceManager() = default;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    AudioSourceManager::~AudioSourceManager()
    {
        Shutdown();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // static
    auto AudioSourceManager::Get() -> AudioSourceManager&
    {
        static AudioSourceManager s_manager;
        return s_manager;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // static
    void AudioSourceManager::Initialize()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioSourceManager::Shutdown()
    {
        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        m_activeAudioInputs.clear();
        m_inactiveAudioInputs.clear();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioSourceManager::CreateSource(Audio::SAudioInputConfig const& sourceConfig) -> bool
    {
        AZStd::unique_ptr<AudioInputSource> ptr = nullptr;
        switch (sourceConfig.m_sourceType)
        {
        case Audio::AudioInputSourceType::PcmFile:
        case Audio::AudioInputSourceType::WavFile:
            // case AudioInputSourceType::OggFile:
            // case AudioInputSourceType::OpusFile:
            {
                if (!sourceConfig.m_sourceFilename.empty())
                {
                    ptr = AZStd::make_unique<AudioInputFile>(sourceConfig);
                }
                break;
            }
        case Audio::AudioInputSourceType::ExternalStream:
            {
                ptr = AZStd::make_unique<AudioInputStreaming>(sourceConfig);
                break;
            }
        default:
            {
                AZ_TracePrintf(
                    "AudioSourceManager",
                    "AudioSourceManager::CreateSource - The type of AudioInputSource requested is "
                    "not supported yet!\n");
                return INVALID_AUDIO_SOURCE_ID;
            }
        }

        if (!ptr || !ptr->IsOk())
        { // this check could change in the future as we add asynch loading.
            return false;
        }

        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        ptr->SetSourceId(sourceConfig.m_sourceId);
        m_inactiveAudioInputs.emplace(sourceConfig.m_sourceId, AZStd::move(ptr));

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioSourceManager::ActivateSource(Audio::TAudioSourceId sourceId, SaInstanceId playingId)
    {
        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        if (m_inactiveAudioInputs.find(sourceId) != m_inactiveAudioInputs.end())
        {
            if (m_activeAudioInputs.find(playingId) == m_activeAudioInputs.end())
            {
                m_inactiveAudioInputs[sourceId]->SetSourceId(sourceId);
                m_activeAudioInputs[playingId] = AZStd::move(m_inactiveAudioInputs[sourceId]);
                m_inactiveAudioInputs.erase(sourceId);

                m_activeAudioInputs[playingId]->OnActivated();
            }
            else
            {
                AZ_TracePrintf(
                    "AudioSourceManager",
                    "AudioSourceManager::ActivateSource - Active source with playing Id %u already "
                    "exists!\n",
                    playingId);
            }
        }
        else
        {
            AZ_TracePrintf(
                "AudioSourceManager",
                "AudioSourceManager::ActivateSource - Source with Id %u not found!\n",
                sourceId);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioSourceManager::DeactivateSource(SaInstanceId playingId)
    {
        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        if (m_activeAudioInputs.find(playingId) != m_activeAudioInputs.end())
        {
            Audio::TAudioSourceId sourceId = m_activeAudioInputs[playingId]->GetSourceId();
            if (m_inactiveAudioInputs.find(sourceId) == m_inactiveAudioInputs.end())
            {
                m_inactiveAudioInputs[sourceId] = AZStd::move(m_activeAudioInputs[playingId]);
                m_activeAudioInputs.erase(playingId);

                // Signal to the audio input source that it was deactivated!  It might unload it's
                // resources.
                m_inactiveAudioInputs[sourceId]->OnDeactivated();

                if (!m_inactiveAudioInputs[sourceId]->IsOk())
                {
                    m_inactiveAudioInputs.erase(sourceId);
                }
            }
            else
            {
                AZ_TracePrintf(
                    "AudioSourceManager",
                    "AudioSourceManager::DeactivateSource - Source with Id %u was already "
                    "inactive!\n",
                    sourceId);
            }
        }
        else
        {
            AZ_TracePrintf(
                "AudioSourceManager",
                "AudioSourceManager::DeactivateSource - Active source with playing Id %u not "
                "found!\n",
                playingId);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void AudioSourceManager::DestroySource(Audio::TAudioSourceId sourceId)
    {
        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        if (m_inactiveAudioInputs.find(sourceId) != m_inactiveAudioInputs.end())
        {
            m_inactiveAudioInputs.erase(sourceId);
        }
        else
        {
            AZ_TracePrintf(
                "AudioSourceManager",
                "AudioSourceManager::DestroySource - No source with Id %u was found!\nDid you call "
                "DeactivateSource first on the playingId??\n",
                sourceId);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto AudioSourceManager::FindPlayingSource(Audio::TAudioSourceId sourceId) -> SaInstanceId
    {
        AZStd::lock_guard<AZStd::mutex> lock(m_inputMutex);

        for (auto& inputPair : m_activeAudioInputs)
        {
            if (inputPair.second->GetSourceId() == sourceId)
            {
                return inputPair.first;
            }
        }

        return InvalidInstanceId;
    }

} // namespace SteamAudio
