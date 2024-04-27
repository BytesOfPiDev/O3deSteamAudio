#pragma once

#include "AzCore/Interface/Interface.h"

#include "phonon.h"

#include "Engine/ISoundEngine.h"

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

    protected:
        void InitMiniAudio();

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
            m_listenerCoordinates; // the world-space position and orientation of the listener

        IPLSimulationSharedInputs m_sharedInputs{};
        AZStd::vector<AudioObject> m_gameObjects{};
    };
} // namespace SteamAudio
