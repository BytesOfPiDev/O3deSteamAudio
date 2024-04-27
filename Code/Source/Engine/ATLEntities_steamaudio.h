#pragma once

#include "ATLEntityData.h"

#include "AzCore/Name/Name.h"
#include "Engine/Id.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    class SATLAudioObjectData_steamaudio : public Audio::IATLAudioObjectData
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SATLAudioObjectData_steamaudio);

        SATLAudioObjectData_steamaudio() = default;
        SATLAudioObjectData_steamaudio(SaGameObjectId id, bool hasPosition)
            : m_hasPosition{ hasPosition }
            , m_id{ id } {};
        ~SATLAudioObjectData_steamaudio() override = default;

        [[nodiscard]] auto HasPosition() const -> bool
        {
            return m_hasPosition;
        }

        [[nodiscard]] auto GetId() const
        {
            return m_id;
        }

        void SetName(AZ::Name name)
        {
            m_name = AZStd::move(name);
        }

        [[nodiscard]] auto GetName() const -> AZ::Name
        {
            return m_name;
        }

    private:
        bool m_hasPosition{};
        SaGameObjectId m_id{};
        AZ::Name m_name;
    };

    struct SATLTriggerImplData_steamaudio : public Audio::IATLTriggerImplData
    {
        SATLTriggerImplData_steamaudio() = default;
        ~SATLTriggerImplData_steamaudio() override = default;
    };

    class SATLListenerData_BopAudio : public Audio::IATLListenerData
    {
        SATLListenerData_BopAudio() = default;
        explicit SATLListenerData_BopAudio(Audio::TAudioObjectID atlId)
            : m_atlId(atlId){};

        [[nodiscard]] auto GetAtlId() const -> Audio::TAudioObjectID
        {
            return m_atlId;
        };

    private:
        Audio::TAudioObjectID m_atlId{};
    };

    class SATLEventData_steamaudio : public Audio::IATLEventData
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SATLEventData_steamaudio);

        SATLEventData_steamaudio() = default;
        SATLEventData_steamaudio(
            SaUniqueId saId,
            Audio::TAudioEventID atlId,
            Audio::EAudioEventState eventState,
            Audio::TAudioSourceId sourceId)
            : m_saId{ saId }
            , m_atlId{ atlId }
            , m_eventState{ eventState }
            , m_sourceId{ sourceId } {};

        ~SATLEventData_steamaudio() override = default;

        [[nodiscard]] auto GetEventState() const -> Audio::EAudioEventState
        {
            return m_eventState;
        }

        [[nodiscard]] auto GetId() const -> SaUniqueId
        {
            return m_saId;
        }

        [[nodiscard]] auto GetAtlEventId() const -> Audio::TAudioEventID
        {
            return m_atlId;
        }

        [[nodiscard]] auto GetSourceId() const -> Audio::TAudioSourceId
        {
            return m_sourceId;
        }

    private:
        Audio::EAudioEventState m_eventState{};
        SaUniqueId m_saId{};
        Audio::TAudioEventID m_atlId{};
        Audio::TAudioSourceId m_sourceId{};
    };

    struct SATLAudioFileEntryData_steamaudio : public Audio::IATLAudioFileEntryData
    {
        AZ_DEFAULT_COPY_MOVE(SATLAudioFileEntryData_steamaudio);

        SATLAudioFileEntryData_steamaudio() = default;
        ~SATLAudioFileEntryData_steamaudio() override = default;
    };

} // namespace SteamAudio
