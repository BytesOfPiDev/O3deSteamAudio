#include "Engine/SaEvent.h"

#include "AzCore/Console/ILogger.h"
#include "Engine/SaSoundSourceAsset.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "Engine/SaEventAsset.h"
#include "Engine/SaSoundInstance.h"

namespace SteamAudio
{
    auto GetNextInstanceId()
    {
        AZStd::atomic<Audio::TAudioTriggerInstanceID> nextInstanceId{ 1 };
        return nextInstanceId++;
    }

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

        if (eventAsset->GetSoundSourceAssets().empty())
        {
            AZLOG_WARN(
                "SaEvent creation - event '%s' has no sound sources", eventAsset.GetHint().c_str());
            return;
        }

        AZStd::ranges::for_each(
            eventAsset->GetSoundSourceAssets(),
            [this](SaSoundSourceAssetPtr const& soundSourceAsset)
            {
                if (!soundSourceAsset.IsReady())
                {
                    AZLOG_ERROR(
                        "SaEvent received a task with bad sound source '%s'",
                        soundSourceAsset.GetHint().c_str());
                    return;
                }

                m_soundInstances.emplace_back(
                    AZStd::make_unique<SoundInstance>(soundSourceAsset->GetSoundSourceName()));
            });
        m_eventState = SaAudioEventState::eAES_NONE;
    }

    SaEvent::~SaEvent()
    {
        AZStd::ranges::for_each(
            m_soundInstances,
            [](auto& soundInstance)
            {
                soundInstance->Stop();
            });

        if (m_eventState != Audio::eAES_NONE)
        {
            StopEvent();
        }

        m_soundInstances.clear();
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
        if (m_soundInstances.empty())
        {
            AZLOG_WARN(
                "SaEvent [%llu] - started, but no sound instances to play",
                static_cast<Audio::TAudioTriggerInstanceID>(m_eventInstanceId));
            return;
        }

        for (auto& soundInstance : m_soundInstances)
        {
            soundInstance->Start();
        }

        AZLOG(
            LOG_SaEvent,
            "SaEvent ['%llu'] - started",
            static_cast<Audio::TAudioTriggerInstanceID>(m_eventInstanceId));
    }

    void SaEvent::StopEvent()
    {
        if (m_eventInstanceId == INVALID_AUDIO_TRIGGER_INSTANCE_ID)
        {
            return;
        }

        m_eventState = SaAudioEventState::eAES_UNLOADING;

        for (auto& soundInstance : m_soundInstances)
        {
            soundInstance->Stop();
        }

        m_eventState = SaAudioEventState::eAES_NONE;

        AZLOG(
            LOG_SaEvent,
            "SaEvent ['%llu'] stopped",
            static_cast<Audio::TAudioTriggerInstanceID>(m_eventInstanceId));
    };

    void SaEvent::SetPosition(Audio::SATLWorldPosition const& worldPosition)
    {
        AZStd::ranges::for_each(
            m_soundInstances,
            [&worldPosition](auto const& soundInstance)
            {
                soundInstance->SetPosition(worldPosition);
            });
    }
}  // namespace SteamAudio
