#pragma once

#include "AudioAllocators.h"
#include "AzCore/std/containers/unordered_map.h"
#include "IAudioInterfacesCommonData.h"
#include "phonon.h"

#include "Engine/Id.h"

namespace SteamAudio
{

    class AudioInputSource
    {
    public:
        AUDIO_IMPL_CLASS_ALLOCATOR(AudioInputSource);

        AudioInputSource() = default;
        virtual ~AudioInputSource() = default;

        virtual void ReadInput(Audio::AudioStreamData const& data) = 0;
        virtual void WriteOutput(IPLAudioBuffer* akBuffer) = 0;

        [[nodiscard]] virtual auto IsOk() const -> bool = 0;
        [[nodiscard]] virtual auto IsFormatValid() const -> bool;

        virtual void OnActivated()
        {
        }
        virtual void OnDeactivated()
        {
        }

        void SetSourceId(Audio::TAudioSourceId sourceId);
        [[nodiscard]] auto GetSourceId() const -> Audio::TAudioSourceId;

    protected:
        Audio::SAudioInputConfig m_config;  ///< Configuration information for the source type.
    };

    class AudioSourceManager
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioSourceManager);
        AudioSourceManager();
        ~AudioSourceManager();

        static auto Get() -> AudioSourceManager&;
        static void Initialize();
        void Shutdown();

        /**
         * CreateSource a new AudioInputSource.
         * Creates an AudioInputSource, based on the SAudioInputConfig and stores it in an inactive
         * state.
         * @param sourceConfig Configuration of the AudioInputSource.
         * @return True if the source was created successfully, false otherwise.
         */
        auto CreateSource(Audio::SAudioInputConfig const& sourceConfig) -> bool;

        /**
         * Activates an AudioInputSource.
         * Moves a source from the inactive state to an active state by assigning an AkPlayingID.
         * @param sourceId ID of the source (returned by CreateSource).
         * @param playingId A playing ID of the source that is now playing in Wwise.
         */
        void ActivateSource(Audio::TAudioSourceId sourceId, SaInstanceId instanceId);

        /**
         * Deactivates an AudioInputSource.
         * Moves a source from the active state back to an inactive state, will happen when an end
         * event callback is recieved.
         * @param playingId Playing ID of the source that ended.
         */
        void DeactivateSource(SaInstanceId instanceId);

        /**
         * Destroy an AudioInputSource.
         * Destroys an AudioInputSource from the manager when it is no longer needed.
         * @param sourceId Source ID of the object to remove.
         */
        void DestroySource(Audio::TAudioSourceId sourceId);

        /**
         * Find the Playing ID of a source.
         * Given a Source ID, check if there are sources in the active state and if so, return their
         * Playing ID.
         * @param sourceId Source ID to look for in the active sources.
         */
        auto FindPlayingSource(Audio::TAudioSourceId sourceId) -> SaInstanceId;

    private:
        AZStd::mutex m_inputMutex;  ///< Callbacks will come from the Wwise event processing thread.

        template<typename KeyType, typename ValueType>
        using AudioInputMap = AZStd::unordered_map<
            KeyType,
            AZStd::unique_ptr<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;

        AudioInputMap<Audio::TAudioSourceId, AudioInputSource>
            m_inactiveAudioInputs;  ///< Sources that haven't started playing yet.
        AudioInputMap<SaInstanceId, AudioInputSource>
            m_activeAudioInputs;  ///< Sources that are currently playing.
    };
}  // namespace SteamAudio
