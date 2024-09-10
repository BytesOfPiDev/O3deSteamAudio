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

        [[nodiscard]] auto GetBinauralEffectSettings() const -> IPLBinauralEffectSettings
        {
            return m_binauralEffectSettings;
        }

        void SetBinauralEffectSettings(IPLBinauralEffectSettings settings)
        {
            m_binauralEffectSettings = settings;
        }

        void Update(float /*deltaTime*/);

    protected:
        void OnTransformChanged(
            const AZ::Transform& /*local*/, const AZ::Transform& /*world*/) override;

        void OnParentTransformWillChange(
            AZ::Transform oldTransform, AZ::Transform newTransform) override;

        auto PushEvent(SaEventId eventId) -> SaEventInstanceId override;
        void PopEvent(SaEventId eventId) override;
        void PopEvent(SaEventInstanceId instanceId) override;

        auto FindEventInstance(SaEventId const& eventId) -> SaEventInstanceId;

    private:
        AZStd::vector<SaEvent> m_events;
        SaGameObjectId m_gameObjectId{};

        IPLSimulationInputs m_inputs{};

        IPLBinauralEffectSettings m_binauralEffectSettings{};
        [[maybe_unused]] IPLAudioBuffer m_inBuffer{};
        [[maybe_unused]] IPLAudioBuffer m_outBuffer{};

        IPLSourceSettings m_sourceSettings{};
        IPLSource m_source{};
        IPLSimulator m_simulator{};
        AZ::EntityId m_entityId{};
    };

}  // namespace SteamAudio

namespace AZStd
{

}
