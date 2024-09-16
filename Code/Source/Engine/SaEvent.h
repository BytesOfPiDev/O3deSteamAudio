#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "Engine/Sound.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/AudioObject.h"
#include "Engine/Id.h"

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
            SaGameObjectId m_objectId{};
        };

        static constexpr auto EmptyEventTask = [](SaGameObjectId) -> void
        {
        };

        using EventFunc = AZStd::function<void(SaGameObjectId)>;

        SaEvent() = delete;
        /// Configures itself based on the given SaEventAsset
        ///
        /// @note If the asset is not already loaded, it will perform a blocking load.
        ///
        /// @param eventAssetId The SaEventAsset to use for configuration
        SaEvent(AZ::Data::AssetId eventAssetId);
        ~SaEvent() = default;

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

    private:
        SaAudioEventState m_eventState{};
        SaEventId m_eventId{};
        SaEventInstanceId m_eventInstanceId{};
        AZStd::vector<SoundSource> m_soundSources{};
        AZStd::vector<SoundInstance> m_soundInstances{};
        AZStd::vector<EventFunc> m_tasks{};
    };
}  // namespace SteamAudio
