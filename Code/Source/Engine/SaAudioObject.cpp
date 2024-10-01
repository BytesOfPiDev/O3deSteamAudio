#include "Engine/SaAudioObject.h"

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
    AZ_CLASS_ALLOCATOR_IMPL(SaAudioObject, Audio::AudioImplAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(SaAudioObject, "AudioObject", SaAudioObjectTypeId);

    SaAudioObject::SaAudioObject() = default;

    SaAudioObject::SaAudioObject(SaAudioObjectId objectId)
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

    SaAudioObject::~SaAudioObject()
    {
        SaAudioObjectRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect(
            static_cast<AZ::EntityId>(m_gameObjectId));
    }

    auto SaAudioObject::FindEventInstance(SaEventId const& eventId) -> SaEventInstanceId
    {
        auto const eventIter{ FindEvent(eventId) };
        return (eventIter != AZStd::end(m_events)) ? eventIter->GetEventInstanceId()
                                                   : InvalidEventInstanceId;
    }

    auto SaAudioObject::PushEvent(SaEventId eventId) -> SaEventInstanceId
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

        auto& event{ m_events.emplace_back(eventAsset.GetId()) };
        event.StartEvent();
        return event.GetEventInstanceId();
    }

    void SaAudioObject::PopEvent(EventIter eventIter)
    {
        if (eventIter == AZStd::end(m_events))
        {
            return;
        }

        eventIter->StopEvent();
        m_events.erase(eventIter);
    }

    void SaAudioObject::PopEventByEventId(SaEventId eventId)
    {
        PopEvent(FindEvent(eventId));
    }

    void SaAudioObject::PopEventByInstanceId(SaEventInstanceId instanceId)
    {
        PopEvent(FindEvent(instanceId));
    }

    void SaAudioObject::Update(float /*deltaTime*/)
    {
    }

    auto SaAudioObject::FindEvent(SaEventId const& eventId) -> EventIter
    {
        return AZStd::ranges::find_if(
            m_events,
            [&eventId](SaEvent const& event) -> bool
            {
                return (event.GetEventId() == eventId);
            });
    }

    auto SaAudioObject::FindEvent(SaEventInstanceId const& instanceId) -> EventIter
    {
        return AZStd::ranges::find_if(
            m_events,
            [&instanceId](SaEvent const& event) -> bool
            {
                return (event.GetEventInstanceId() == instanceId);
            });
    }

    void SaAudioObject::SetPosition(Audio::SATLWorldPosition const& worldPosition)
    {
        AZStd::ranges::for_each(
            m_events,
            [&worldPosition](SaEvent& event)
            {
                event.SetPosition(worldPosition);
            });
    }
}  // namespace SteamAudio
