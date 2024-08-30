#include "Engine/SaEvent.h"

#include "AzCore/Console/ILogger.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/AudioEventBus.h"
#include "Engine/Id.h"
#include "Engine/SaEventAsset.h"
#include "Engine/Sound.h"
#include "Engine/SoundConfig.h"
#include "Engine/Tasks/Task.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    struct SoundEventTask : public ITaskInstance
    {
        AZ_DISABLE_COPY_MOVE(SoundEventTask);
        AZ_RTTI_WITH_NAME(
            SoundEventTask, "SoundTask", "3C0EBC61-6BB8-401B-8F1E-C87C9194B375", ITaskInstance);
        SoundEventTask(SoundTaskConfig const& config)
            : m_source{ config.m_asset,
                        AZ::Name{ AZ::Uuid::CreateRandom().ToFixedString().c_str() } }
        {
            auto const& result{ ma_sound_init_from_file(
                Util::GetMaEngine(), m_source.GetName().GetCStr(), 0, nullptr, nullptr, &m_sound) };

            AZ_Error(
                TYPEINFO_Name(),
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

        ~SoundEventTask() override = default;

        void StartTask() override
        {
            if (!ma_sound_get_data_source(&m_sound))
            {
                return;
            }
            ma_sound_start(&m_sound);
        }

        void StopTask() override
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

    SaEvent::SaEvent() = default;

    SaEvent::SaEvent(AZ::Data::AssetId eventAssetId, SaGameObjectId objectId)
        : m_objectId{ objectId }
    {
        auto eventAsset{ AZ::Data::Asset<SaEventAsset>{ eventAssetId,
                                                        AZ::AzTypeInfo<SaEventAsset>::Uuid() } };
        eventAsset.QueueLoad() ? eventAsset.BlockUntilLoadComplete()
                               : AZ::Data::AssetData::AssetStatus{};

        AZ_Error("SaEvent", eventAsset, "Unable to get event asset w/ id '%s'", eventAssetId);
        AZ_Error(
            "SaEvent",
            m_objectId != INVALID_AUDIO_OBJECT_ID,
            "Invalid audio object id: %zu",
            objectId);

        if (!eventAsset || m_objectId == INVALID_AUDIO_OBJECT_ID)
        {
            return;
        }

        SaEventRequestBus::Handler::BusConnect(m_objectId);

        AZStd::ranges::for_each(
            eventAsset->GetTasksConfigs(),
            [this](TaskDefinition& taskDef)
            {
                auto const& config = taskDef.GetConfig();
                if (config.is<SoundTaskConfig>())
                {
                    auto const& soundTaskConfig{ AZStd::any_cast<SoundTaskConfig const&>(config) };
                    AZ_Error("SaEvent", false, "TESTING: Got sound task config!");

                    m_tasks.emplace_back(aznew SoundEventTask{ soundTaskConfig });
                }
            });
    }

    SaEvent::~SaEvent()
    {
        SaEventRequestBus::Handler::BusDisconnect();
    }

    void SaEvent::Update(float)
    {
    }

    void SaEvent::StartEvent()
    {
        auto const* const objectId = SaEventRequestBus::GetCurrentBusId();
        if (!objectId)
        {
            return;
        }

        AZLOG(LOG_SaEvent, "SaEvent::Start(objectId: %llu)", *objectId);
        AZStd::ranges::for_each(
            m_tasks,
            [](auto& task)
            {
                task ? task->StartTask() : void();
            });
    }

    void SaEvent::StopEvent()
    {
        auto const* const objectId = SaEventRequestBus::GetCurrentBusId();
        if (!objectId)
        {
            return;
        }

        AZLOG(LOG_SaEvent, "SaEvent::Stop(objectId: %llu)", *objectId);
        AZStd::ranges::for_each(
            m_tasks,
            [](auto& task)
            {
                task ? task->StopTask() : void();
            });
    };

    void SaEvent::Prepare() const {

    };

    void SaEvent::StartEventById(SaEventId id)
    {
        if (id != m_eventId)
        {
            AZLOG_INFO("StartEventById: Id %llu is not ours - '%llu'", id, m_eventId);
            return;
        }

        AZLOG_INFO("StartEventById: %llu", id);

        StartEvent();
    }

    void SaEvent::StopEventById(SaEventId id)
    {
        if (id != m_eventId)
        {
            return;
        }

        StopEvent();
    }
}  // namespace SteamAudio
