#pragma once

#include <AudioAllocators.h>

#include "AzCore/Component/TransformBus.h"
#include "AzCore/base.h"

#include "Engine/Id.h"
#include "Engine/SaAudioObjectBus.h"
#include "Engine/SaEvent.h"
#include "phonon.h"

namespace SteamAudio
{
    class SaEvent;

    class AudioObject
        : protected SaAudioObjectRequestBus::Handler
        , protected AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(AudioObject);
        AZ_TYPE_INFO_WITH_NAME_DECL(AudioObject);

        AudioObject();
        explicit AudioObject(SaGameObjectId objectId);
        AudioObject(AZ::EntityId entityId, IPLSimulator simulator);

        ~AudioObject() override;

        explicit operator AZ::u64() const
        {
            return static_cast<AZ::u64>(m_gameObjectId);
        }

        [[nodiscard]] auto GetUniqueId() const -> SaGameObjectId
        {
            return m_gameObjectId;
        }

        void Update(float /*deltaTime*/);

    protected:
        using EventIter = AZStd::vector<SaEvent>::iterator;

        auto PushEvent(SaEventId eventId) -> SaEventInstanceId override;
        void PopEventByEventId(SaEventId eventId) override;
        void PopEventByInstanceId(SaEventInstanceId instanceId) override;

        auto FindEventInstance(SaEventId const& eventId) -> SaEventInstanceId;
        auto FindEvent(SaEventId const& eventId) -> EventIter;
        auto FindEvent(SaEventInstanceId const& instanceId) -> EventIter;

        void PopEvent(EventIter);

    private:
        AZStd::vector<SaEvent> m_events;
        SaGameObjectId m_gameObjectId{};
        AZ::EntityId m_entityId{};
    };

}  // namespace SteamAudio

namespace AZStd
{

}
