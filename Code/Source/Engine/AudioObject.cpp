#include "Engine/AudioObject.h"
#include "AudioAllocators.h"
#include "AzCore/Memory/Memory_fwd.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(AudioObject, Audio::AudioImplAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(AudioObject, "AudioObject", "7E588CE9-9C24-4C52-BE3D-ADC704080278");

    AudioObject::AudioObject()
    {
        m_sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
    }

    AudioObject::AudioObject(AZ::EntityId entityId, IPLSimulator simulator)
        : m_entityId{ entityId }
        , m_simulator(simulator)
    {
        AZ::EntityId d;
        if (m_gameObjectId != AZ::EntityId::InvalidEntityId)
        {
            AZ::TransformNotificationBus::Handler::BusConnect(
                static_cast<AZ::EntityId>(m_gameObjectId));
        }

        IPLCoordinateSpace3 sourceCoords{};
        AZ::Transform gameTransform{};
        AZ::TransformBus::EventResult(
            gameTransform,
            static_cast<AZ::EntityId>(m_gameObjectId),
            &AZ::TransformBus::Events::GetWorldTM);

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
        AZ::TransformNotificationBus::Handler::BusDisconnect(
            static_cast<AZ::EntityId>(m_gameObjectId));
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

}  // namespace SteamAudio
