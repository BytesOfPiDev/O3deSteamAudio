#pragma once

#include <phonon.h>

#include "AudioAllocators.h"

#include "AzCore/std/any.h"
#include "Engine/AudioEvent.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/AudioObject.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/Id.h"

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

        auto IsInitialized() const -> bool override
        {
            return m_initialized;
        }

        auto Initialize() -> EngineNullOutcome override;

        auto Shutdown() -> EngineNullOutcome override;

        void Update(float deltaTime);

        auto RegisterAudioObject(SaGameObjectId const& /*audioObject*/)
            -> EngineNullOutcome override;

        auto ReportEvent(StartEventData const&) -> EngineNullOutcome override;

    protected:
        auto InitMiniAudio() -> EngineNullOutcome;
        auto ShutdownMiniAudio() -> EngineNullOutcome;

        void LoadNativeEvents();
        void LoadEventAssets();

        [[nodiscard]] auto FindEvent(SaEventId eventId) const
            -> AZ::Outcome<AZ::Data::Asset<SaEventAsset>, AZStd::string>;
        auto FindObject(SaGameObjectId id) -> AZ::Outcome<AudioObject*>;

    private:
        IPLContextSettings m_contextSettings{};

        template<typename KeyType, typename ValueType>
        using GameObjectMap = AZStd::unordered_map<
            KeyType,
            AZStd::unique_ptr<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;
        GameObjectMap<SaGameObjectId, AudioObject> m_registeredObjects{};

        template<typename KeyType, typename ValueType>
        using EventMap = AZStd::unordered_map<
            KeyType,
            AZStd::unique_ptr<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;
        EventMap<SaEventId, SaEvent> m_activeEvents{};

        template<typename KeyType, typename ValueType>
        using EventAssetMap = AZStd::unordered_map<
            KeyType,
            AZ::Data::Asset<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;
        EventAssetMap<SaEventId, SaEventAsset> m_eventAssets{};

        AZStd::any m_engine{};

        bool m_initialized{};
    };
}  // namespace SteamAudio
