#include "Engine/AudioObject.h"

namespace SteamAudio
{

    AudioObject::AudioObject()
    {
        m_sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
    }

    AudioObject::AudioObject(SaGameObjectId objectId, IPLSimulator simulator)
        : m_gameObjectId(objectId)
        , m_simulator(simulator)
    {
        if (m_gameObjectId.IsValid())
        {
            AZ::TransformNotificationBus::Handler::BusConnect(m_gameObjectId);
        }

        IPLCoordinateSpace3 sourceCoords{};
        AZ::Transform gameTransform{};
        AZ::TransformBus::EventResult(
            gameTransform, m_gameObjectId, &AZ::TransformBus::Events::GetWorldTM);

        auto translation{ gameTransform.GetTranslation() };
        sourceCoords.origin.x = translation.GetX();
        sourceCoords.origin.y = translation.GetY();
        sourceCoords.origin.z = translation.GetZ();

        iplSourceCreate(m_simulator, &m_sourceSettings, &m_source);
        iplSourceAdd(m_source, m_simulator);
        iplSimulatorCommit(m_simulator);

        m_inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
        m_inputs.directFlags = static_cast<decltype(m_inputs.directFlags)>(
            IPL_DIRECTSIMULATIONFLAGS_OCCLUSION | IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
        m_inputs.source = sourceCoords;
        m_inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
    }

    AudioObject::~AudioObject()
    {
        AZ::TransformNotificationBus::Handler::BusDisconnect(m_gameObjectId);
        iplSourceRemove(m_source, m_simulator);
        iplSimulatorCommit(m_simulator);
    }

    void AudioObject::OnTransformChanged(
        const AZ::Transform& /*local*/, const AZ::Transform& /*world*/)
    {
        IPLCoordinateSpace3 sourceCoords{};

        m_inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
        m_inputs.directFlags = static_cast<decltype(m_inputs.directFlags)>(
            IPL_DIRECTSIMULATIONFLAGS_OCCLUSION | IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
        m_inputs.source = sourceCoords;
        m_inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;

        iplSourceSetInputs(m_source, IPL_SIMULATIONFLAGS_DIRECT, &m_inputs);
    }

    void AudioObject::OnParentTransformWillChange(
        AZ::Transform /*oldTransform*/, AZ::Transform /*newTransform*/)
    {
    }

} // namespace SteamAudio
