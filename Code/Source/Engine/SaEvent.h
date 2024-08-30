#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "Engine/AudioObject.h"
#include "Engine/Id.h"
#include "Engine/Tasks/Task.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/AudioEventBus.h"

namespace SteamAudio
{
    using SaAudioEventState = Audio::EAudioEventState;

    class SaEvent : protected SaEventRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY(SaEvent);

        struct EventConfig
        {
            AZ::Data::AssetId m_eventAssetId{};
            SaGameObjectId m_objectId{};
        };

        static constexpr auto EmptyEventTask = [](SaGameObjectId) -> void
        {
        };

        SaEvent();
        /// Configures itself based on the given SaEventAsset
        ///
        /// @note If the asset is not already loaded, it will perform a blocking load.
        ///
        /// @param eventAssetId The SaEventAsset to use for configuration
        SaEvent(AZ::Data::AssetId eventAssetId, SaGameObjectId objectId);
        ~SaEvent() override;

        void Prepare() const;
        void Update(float);

        [[nodiscard]] auto GetEventState() const -> SaAudioEventState
        {
            return m_eventState;
        }

        [[nodiscard]] auto GetEventId() const -> SaEventId
        {
            return m_eventId;
        }

    protected:
        void StartEvent();
        void StopEvent();

        void StartEventById(SaEventId) override;
        void StopEventById(SaEventId) override;

    private:
        AZStd::vector<AZStd::unique_ptr<ITaskInstance>> m_tasks{};
        SaEventId m_eventId{};
        SaGameObjectId m_objectId{};

        SaAudioEventState m_eventState{};
    };
}  // namespace SteamAudio
