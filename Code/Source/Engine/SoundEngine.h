#pragma once

#include <phonon.h>

#include "AudioAllocators.h"
#include "AzCore/Interface/Interface.h"

#include "Engine/AudioEvent.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/AudioObject.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/Id.h"

#include "miniaudio.h"

namespace SteamAudio
{
    class SteamAudioEngine : public AZ::Interface<ISoundEngine>::Registrar
    {
    public:
        auto Initialize() -> EngineNullOutcome override;

        auto Shutdown() -> EngineNullOutcome override;

        void Update(float deltaTime);

        auto RegisterAudioObject(SaGameObjectId const& /*audioObject*/)
            -> EngineNullOutcome override;

        void AddEvent(SaEventId eventId, AZStd::unique_ptr<SteamAudio::SaEvent> /*event*/) override;
        auto ReportEvent(StartEventData const&) -> EngineNullOutcome override;

    protected:
        auto InitMiniAudio() -> EngineNullOutcome;

        void LoadNativeEvents();
        void LoadEventAssets();

        [[nodiscard]] auto FindEvent(SaEventId eventId) const
            -> AZ::Outcome<SaEvent*, AZStd::string>;
        auto FindObject(SaGameObjectId id) -> AZ::Outcome<AudioObject*>;

    private:
        IPLContextSettings m_contextSettings{};
        IPLContext m_context{};
        IPLHRTFSettings m_hrtfSettings{};
        IPLAudioSettings m_audioSettings{};

        IPLHRTF m_hrtf{};

        IPLSceneSettings m_sceneSettings{};
        IPLScene m_scene{};
        IPLSimulator m_simulator{};
        IPLSimulationSettings m_simulationSettings{};

        IPLCoordinateSpace3
            m_listenerCoordinates;  // the world-space position and orientation of the listener

        IPLSimulationSharedInputs m_sharedInputs{};

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
        EventMap<SaEventId, SaEvent> m_events{};

        template<typename KeyType, typename ValueType>
        using EventAssetMap = AZStd::unordered_map<
            KeyType,
            AZ::Data::Asset<ValueType>,
            AZStd::hash<KeyType>,
            AZStd::equal_to<KeyType>,
            Audio::AudioImplStdAllocator>;
        EventAssetMap<SaEventId, SaEventAsset> m_eventAssets{};

        ma_device_config m_maDeviceConfig{};

        AZStd::function<void(
            ma_device* pDevice, void* pOutput, void const* pInput, ma_uint32 frameCount)>
            m_maDataCallback{};
    };
}  // namespace SteamAudio
