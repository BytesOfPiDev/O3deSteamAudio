#pragma once

#include <AzCore/std/utils.h>

#include "ATLEntityData.h"
#include "AzCore/Name/Name.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "IAudioSystem.h"

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

    class SATLTriggerImplData_steamaudio : public Audio::IATLTriggerImplData
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SATLTriggerImplData_steamaudio);

        SATLTriggerImplData_steamaudio() = default;
        SATLTriggerImplData_steamaudio(AZStd::string_view triggerName)
            : m_triggerName{ AZ::Name{ triggerName } }
            , m_implEventId{ Audio::AudioStringToID<SaEventId>(triggerName.data()) } {};

        SATLTriggerImplData_steamaudio(AZ::Name triggerName, SaEventId implEventId)
            : m_triggerName{ AZStd::move(triggerName) }
            , m_implEventId{ implEventId }
        {
        }

        ~SATLTriggerImplData_steamaudio() override = default;

        [[nodiscard]] auto GetImplEventId() const -> SaEventId
        {
            return m_implEventId;
        }

    private:
        AZ::Name m_triggerName{};
        SaEventId m_implEventId{};
    };

    class SATLListenerData_BopAudio : public Audio::IATLListenerData
    {
        SATLListenerData_BopAudio() = default;

        explicit SATLListenerData_BopAudio(Audio::TAudioObjectID atlId)
            : m_atlId(atlId)
        {
        }

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
        explicit SATLEventData_steamaudio(Audio::TAudioEventID atlEventId)
            : m_atlEventId{ atlEventId }
        {
        }

        ~SATLEventData_steamaudio() override = default;

        [[nodiscard]] auto GetEventState() const -> Audio::EAudioEventState
        {
            return m_atlEventState;
        }

        [[nodiscard]] auto GetEngineId() const -> SaId
        {
            return m_engineId;
        }

        [[nodiscard]] auto GetAtlEventId() const -> Audio::TAudioEventID
        {
            return m_atlEventId;
        }

        [[nodiscard]] auto GetSourceId() const -> Audio::TAudioSourceId
        {
            return m_audioSourceId;
        }

        void ChangeAtlEventState(Audio::EAudioEventState newState)
        {
            m_atlEventState = newState;
        }

        [[nodiscard]] auto GetInstanceId() const -> SaEventInstanceId
        {
            return m_triggerInstanceId;
        }

        void SetInstanceId(SaEventInstanceId instanceId)
        {
            m_triggerInstanceId = instanceId;
        }

    private:
        Audio::EAudioEventState m_atlEventState{};
        Audio::TAudioEventID m_atlEventId{};
        Audio::TAudioSourceId m_audioSourceId{};
        SaEventInstanceId m_triggerInstanceId{};
        SaId m_engineId{};
    };

    struct SATLAudioFileEntryData_steamaudio : public Audio::IATLAudioFileEntryData
    {
        AZ_DEFAULT_COPY_MOVE(SATLAudioFileEntryData_steamaudio);

        SATLAudioFileEntryData_steamaudio() = default;
        ~SATLAudioFileEntryData_steamaudio() override = default;
    };

}  // namespace SteamAudio
