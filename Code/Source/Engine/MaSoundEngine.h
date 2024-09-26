#pragma once

#include <phonon.h>

#include "AudioAllocators.h"
#include "AzCore/std/any.h"

#include "Engine/AudioObject.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/Id.h"
#include "Engine/SaRegisteredEvent.h"
#include "Engine/SoundSourceLoader.h"
#include "IAudioInterfacesCommonData.h"

extern "C" {
struct ma_engine;
struct ma_sound;
}

namespace SteamAudio
{
    class MaSoundEngine : public SoundEngineRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(MaSoundEngine);

        MaSoundEngine();
        ~MaSoundEngine() override;

        [[nodiscard]] auto IsInitialized() const -> bool override
        {
            return m_initialized;
        }

        /// Attempts to initialize the audio engine
        ///
        /// NOTE: The ma_engine environment variable is not available until this succeeds
        auto Initialize() -> EngineNullOutcome override;

        auto Shutdown() -> EngineNullOutcome override;

        void Update(float deltaTime);

        auto RegisterAudioObject(SaGameObjectId const& /*audioObject*/)
            -> EngineNullOutcome override;

        auto ReportEvent(StartEventData const&) -> SaEventInstanceId override;

    protected:
        auto InitMiniAudio() -> EngineNullOutcome;
        auto ShutdownMiniAudio() -> EngineNullOutcome;

        void LoadNativeEvents();
        void LoadEventAssets();
        void LoadSounds();

        void ActivateEvent(SaGameObjectId, AZ::Name);
        void ActivateEvent(SaGameObjectId, AZ::Data::AssetId);

    private:
        IPLContextSettings m_contextSettings{};

        template<typename KeyType, typename ValueType>
        using GameObjectMap = AZStd::unordered_map<
            KeyType,
            AZStd::unique_ptr<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;
        GameObjectMap<SaGameObjectId, SaAudioObject> m_registeredObjects{};

        AZStd::vector<AZStd::unique_ptr<SaRegisteredEvent>> m_registeredEvents{};

        AZStd::any m_maEngine{};
        bool m_initialized{};

        SoundSourceLoader m_soundLoader{};
        AZStd::optional<SaAudioObject> m_globalAudioObject{ AZStd::nullopt };
    };
}  // namespace SteamAudio
