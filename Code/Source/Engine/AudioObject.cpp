#include "Engine/AudioObject.h"

#include "AudioAllocators.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "Engine/SaAudioObjectBus.h"
#include "Engine/SaRegisteredEventBus.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(AudioObject, Audio::AudioImplAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(AudioObject, "AudioObject", SaAudioObjectTypeId);

    AudioObject::AudioObject() = default;

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
            AZLOG_ERROR(
                "AudioObject created with object id '%llu' that is already in use!",
                m_gameObjectId);
            return;
        }

        SaAudioObjectRequestBus::Handler::BusConnect(m_gameObjectId);
    }

    AudioObject::~AudioObject()
    {
        SaAudioObjectRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect(
            static_cast<AZ::EntityId>(m_gameObjectId));
    }

    auto AudioObject::FindEventInstance(SaEventId const& eventId) -> SaEventInstanceId
    {
        auto const eventIter{ FindEvent(eventId) };
        return (eventIter != AZStd::end(m_events)) ? eventIter->GetEventInstanceId()
                                                   : InvalidEventInstanceId;
    }

    auto AudioObject::PushEvent(SaEventId eventId) -> SaEventInstanceId
    {
        AZ::Data::Asset<SaEventAsset> eventAsset{};
        SaRegisteredEventRequestBus::EventResult(
            eventAsset, eventId, &SaRegisteredEventBusRequests::GetEvent);
        if (!eventAsset)
        {
            AZLOG_ERROR(
                "Audio object '%llu' is unable to push event using id '%llu': null asset "
                "retrieved. Asset Hint: '%s'",
                static_cast<Audio::TAudioObjectID>(m_gameObjectId),
                eventId.GetValue(),
                eventAsset.GetHint().c_str());
            return InvalidEventInstanceId;
        }

        auto const& event{ m_events.emplace_back(eventAsset.GetId()) };
        AZLOG_ERROR(
            "AudioObject::PushEvent w/ InstanceId: '%llu'", event.GetEventInstanceId().m_value);
        return event.GetEventInstanceId();
    }

    void AudioObject::PopEvent(EventIter eventIter)
    {
        if (eventIter == AZStd::end(m_events))
        {
            return;
        }

        eventIter->StopEvent();
        m_events.erase(eventIter);
    }

    void AudioObject::PopEventByEventId(SaEventId eventId)
    {
        PopEvent(FindEvent(eventId));
    }

    void AudioObject::PopEventByInstanceId(SaEventInstanceId instanceId)
    {
        PopEvent(FindEvent(instanceId));
    }

    void AudioObject::Update(float /*deltaTime*/)
    {
    }

    auto AudioObject::FindEvent(SaEventId const& eventId) -> EventIter
    {
        return AZStd::ranges::find_if(
            m_events,
            [&eventId](SaEvent const& event) -> bool
            {
                return (event.GetEventId() == eventId);
            });
    }

    auto AudioObject::FindEvent(SaEventInstanceId const& instanceId) -> EventIter
    {
        return AZStd::ranges::find_if(
            m_events,
            [&instanceId](SaEvent const& event) -> bool
            {
                return (event.GetEventInstanceId() == instanceId);
            });
    }
}  // namespace SteamAudio
