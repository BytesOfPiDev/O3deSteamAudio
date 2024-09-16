#include "Engine/SaEvent.h"

#include "AzCore/Console/ILogger.h"
#include "AzCore/PlatformDef.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "Engine/SaEventAsset.h"
#include "Engine/Sound.h"
#include "Engine/SoundConfig.h"
#include "Engine/Tasks/Task.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{

    auto GetNextInstanceId()
    {
        AZStd::atomic<Audio::TAudioTriggerInstanceID> nextInstanceId{ 1 };
        return nextInstanceId++;
    }

    struct SoundEventTask
    {
        AZ_DISABLE_COPY_MOVE(SoundEventTask);
        AZ_TYPE_INFO_WITH_NAME(SoundEventTask, "SoundTask", "3C0EBC61-6BB8-401B-8F1E-C87C9194B375");

        SoundEventTask() = default;

        SoundEventTask(SoundTaskConfig const& config)
            : m_source{ config.m_asset,
                        AZ::Name{ AZ::Uuid::CreateRandom().ToFixedString().c_str() } }
        {
            auto const& result{ ma_sound_init_from_file(
                Util::GetMaEngine(), m_source.GetName().GetCStr(), 0, nullptr, nullptr, &m_sound) };

            AZ_Error(
                AZ_FUNCTION_SIGNATURE,
                result == MA_SUCCESS,
                "'%s' is not a registered sound.",
                m_source.GetName().GetCStr());
            if (result != MA_SUCCESS)
            {
                return;
            }

            ma_sound_set_volume(&m_sound, config.m_volume);
            ma_sound_set_looping(&m_sound, config.m_loop);
        };

        ~SoundEventTask() = default;

        void StartTask()
        {
            if (!ma_sound_get_data_source(&m_sound))
            {
                return;
            }

            ma_sound_start(&m_sound);
        }

        void StopTask()
        {
            if (!ma_sound_get_data_source(&m_sound))
            {
                return;
            }
            ma_sound_stop(&m_sound);
        }

        ma_sound m_sound{};
        SoundSource m_source{};
    };

    SaEvent::SaEvent(AZ::Data::AssetId eventAssetId)
        : m_eventState(SaAudioEventState::eAES_LOADING)
        , m_eventInstanceId(GetNextInstanceId())
    {
        auto eventAsset{ AZ::Data::Asset<SaEventAsset>{ eventAssetId,
                                                        AZ::AzTypeInfo<SaEventAsset>::Uuid() } };
        eventAsset.QueueLoad() ? eventAsset.BlockUntilLoadComplete()
                               : AZ::Data::AssetData::AssetStatus{};

        AZ_Error("SaEvent", eventAsset, "Unable to get event asset w/ id '%s'", eventAssetId);
        AZ_Error("SaEvent", m_eventInstanceId != INVALID_AUDIO_TRIGGER_INSTANCE_ID, "Invalid SaId");

        if (!eventAsset || m_eventInstanceId == INVALID_AUDIO_TRIGGER_INSTANCE_ID)
        {
            return;
        }

        AZStd::ranges::for_each(
            eventAsset->GetTasksDefinitions(),
            [this](TaskDefinition& taskDef)
            {
                [[maybe_unused]] auto const& config = taskDef.GetConfig();
                AZ_Error("SaEvent", false, "TESTING: Got sound task config!");

                AZ::Name randomId{ AZ::Uuid::CreateRandom().ToFixedString().c_str() };
                m_soundSources.emplace_back(taskDef.GetConfig().m_asset, randomId);
                m_soundInstances.emplace_back(randomId, true);

                m_tasks.emplace_back(
                    [](SaGameObjectId)
                    {
                    });
            });
        m_eventState = SaAudioEventState::eAES_NONE;
    }

    void SaEvent::Update(float)
    {
        if (m_eventState != SaAudioEventState::eAES_PLAYING)
        {
            return;
        }
    }

    void SaEvent::StartEvent()
    {
        if (m_eventInstanceId == INVALID_AUDIO_TRIGGER_INSTANCE_ID)
        {
            AZLOG_INFO("Unable to start event due to invalid event instance id");
            return;
        }

        m_eventState = SaAudioEventState::eAES_PLAYING;
    }

    void SaEvent::StopEvent()
    {
        if (m_eventInstanceId == INVALID_AUDIO_TRIGGER_INSTANCE_ID)
        {
            return;
        }

        m_eventState = SaAudioEventState::eAES_UNLOADING;
        AZLOG(
            LOG_SaEvent,
            "SaEvent::Stop(objectId: %llu)",
            static_cast<Audio::TAudioTriggerInstanceID>(m_eventInstanceId));

        m_soundInstances.clear();

        m_eventState = SaAudioEventState::eAES_NONE;
    };
}  // namespace SteamAudio
