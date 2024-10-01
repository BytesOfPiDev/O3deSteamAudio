#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "Engine/SaAudioObject.h"
#include "Engine/SaSoundInstance.h"

namespace SteamAudio
{
    using SaAudioEventState = Audio::EAudioEventState;

    class SaEvent
    {
    public:
        AZ_DISABLE_COPY(SaEvent);

        struct EventConfig
        {
            AZ::Data::AssetId m_eventAssetId{};
            SaAudioObjectId m_objectId{};
        };

        static constexpr auto EmptyEventTask = [](SaAudioObjectId) -> void
        {
        };

        using EventFunc = AZStd::function<void(SaAudioObjectId)>;

        SaEvent() = delete;
        /// Configures itself based on the given SaEventAsset
        ///
        /// @note If the asset is not already loaded, it will perform a blocking load.
        ///
        /// @param eventAssetId The SaEventAsset to use for configuration
        SaEvent(AZ::Data::AssetId eventAssetId);
        ~SaEvent();

        void Update(float);

        [[nodiscard]] auto GetEventState() const -> SaAudioEventState
        {
            return m_eventState;
        }

        [[nodiscard]] auto GetEventId() const -> SaEventId
        {
            return m_eventId;
        }

        [[nodiscard]] auto GetEventInstanceId() const -> SaEventInstanceId
        {
            return m_eventInstanceId;
        };

        void StartEvent();
        void StopEvent();
        void SetPosition(Audio::SATLWorldPosition const& worldPosition);

    private:
        SaAudioEventState m_eventState{};
        SaEventId m_eventId{};
        SaEventInstanceId m_eventInstanceId{};
        AZStd::vector<AZStd::unique_ptr<SoundInstance>> m_soundInstances{};
        AZStd::vector<EventFunc> m_tasks{};
    };
}  // namespace SteamAudio
