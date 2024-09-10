#include "Engine/AudioObject.h"
#include "AudioAllocators.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/EBus/Results.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/std/ranges/ranges_algorithm.h"
#include "Engine/AudioEventBus.h"
#include "Engine/Id.h"
#include "Engine/SaAudioObjectBus.h"
#include "Engine/SaRegisteredEventBus.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(AudioObject, Audio::AudioImplAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(AudioObject, "AudioObject", "7E588CE9-9C24-4C52-BE3D-ADC704080278");

    AudioObject::AudioObject()
    {
        m_sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
    }

    AudioObject::AudioObject(SaGameObjectId objectId)
        : m_gameObjectId{ objectId }
    {
        if (m_gameObjectId == INVALID_AUDIO_OBJECT_ID)
        {
            AZLOG_ERROR("AudioObject created with invalid object id!");
            return;
        }

        if (SaAudioObjectRequestBus::HasHandlers(m_gameObjectId))
        {
            AZLOG_ERROR("AudioObject created with object id that is already in use!");
            return;
        }

        SaAudioObjectRequestBus::Handler::BusConnect(m_gameObjectId);
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
        SaAudioObjectRequestBus::Handler::BusDisconnect();
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

    auto AudioObject::FindEventInstance(SaEventId const& eventId) -> SaEventInstanceId
    {
        return {};
    }

    auto AudioObject::PushEvent(SaEventId eventId) -> SaEventInstanceId
    {
        AZ::Data::Asset<SaEventAsset> eventAsset{};
        SaRegisteredEventRequestBus::EventResult(
            eventAsset, eventId, &SaRegisteredEventBusRequests::GetEvent);
        if (!eventAsset)
        {
            AZLOG_ERROR(
                "Audio object '%llu' is unable to push event using id '%llu': null asset retrieved",
                static_cast<Audio::TAudioObjectID>(m_gameObjectId),
                static_cast<Audio::TAudioTriggerImplID>(eventId));
            return InvalidEventInstanceId;
        }

        auto const& event{ m_events.emplace_back(eventAsset.GetId()) };
        return event.GetEventInstanceId();
    }
    void AudioObject::PopEvent(SaEventId eventId)
    {
    }
    void AudioObject::PopEvent(SaEventInstanceId instanceId)
    {
    }
    void AudioObject::Update(float /*deltaTime*/)
    {
    }
}  // namespace SteamAudio
