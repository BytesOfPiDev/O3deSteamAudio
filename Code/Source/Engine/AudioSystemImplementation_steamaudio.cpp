#include "Engine/AudioSystemImplementation_steamaudio.h"

#include "ATLEntityData.h"
#include "AudioAllocators.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Settings/SettingsRegistry.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "Engine/Id.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"
#include "IAudioSystemImplementation.h"

#include "Engine/ATLEntities_steamaudio.h"
#include "Engine/Common_steamaudio.h"
#include "Engine/Configuration.h"

namespace SteamAudio
{
    char const* const AudioSystemImpl_steamaudio::SteamAudioImplSubPath = "steamaudio/";

    AZ_TYPE_INFO_WITH_NAME_IMPL(
        AudioSystemImpl_steamaudio, "Steam Audio System", "{83ACADC6-A911-443D-A4DC-1BC9D557F106}");
    AZ_RTTI_NO_TYPE_INFO_IMPL(AudioSystemImpl_steamaudio, Audio::AudioSystemImplementation);

    AudioSystemImpl_steamaudio::AudioSystemImpl_steamaudio()
    {
        SetPaths();

        Audio::AudioSystemImplementationRequestBus::Handler::BusConnect();
        Audio::AudioSystemImplementationNotificationBus::Handler::BusConnect();
    }

    AudioSystemImpl_steamaudio::~AudioSystemImpl_steamaudio()
    {
        Audio::AudioSystemImplementationRequestBus::Handler::BusDisconnect();
        Audio::AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
    }

    void AudioSystemImpl_steamaudio::SetPaths()
    {
        auto* const settingsRegistry{ AZ::SettingsRegistry::Get() };

        if (settingsRegistry == nullptr)
        {
            AZ_Error(
                TYPEINFO_Name(), false, "SteamAudio could not get the settings registry instance.");

            return;
        }

        if (settingsRegistry->Get(m_language, Settings::LanguageKey))
        {
            m_localizedSoundBankFolder = { m_soundBankFolder / m_language };
        }
        else
        {
            AZLOG_INFO("SteamAudio found no value found at key: \"%s\".", Settings::LanguageKey);
        };

        if (AZ::IO::FileIOBase::GetInstance() == nullptr)
        {
            AZLOG_ERROR(
                "ASI is unable to set the soundbank folder because due to no FileIO instance.");

            return;
        }

        m_soundBankFolder = BanksAlias;
    }

    void AudioSystemImpl_steamaudio::OnAudioSystemLoseFocus()
    {
    }

    void AudioSystemImpl_steamaudio::OnAudioSystemGetFocus()
    {
    }

    void AudioSystemImpl_steamaudio::OnAudioSystemMuteAll()
    {
    }

    void AudioSystemImpl_steamaudio::OnAudioSystemUnmuteAll()
    {
    }

    void AudioSystemImpl_steamaudio::OnAudioSystemRefresh()
    {
    }

    void AudioSystemImpl_steamaudio::Update(float const updateIntervalMS)
    {
        AZ_UNUSED(updateIntervalMS);
    }

    auto AudioSystemImpl_steamaudio::Initialize() -> Audio::EAudioRequestStatus
    {
        m_engine = AZ::Interface<ISoundEngine>::Get();

        if (!m_engine)
        {
            AZ_Error(
                TYPEINFO_Name(),
                false,
                "Initialization failed. The SteamAudio sound engine is not available.\n");

            return Audio::EAudioRequestStatus::Failure;
        }

        if (auto outcome{ m_engine->Initialize() }; !outcome.IsSuccess())
        {
            AZ_Error(
                TYPEINFO_Name(),
                false,
                "Initialization failed: [%s].\n",
                outcome.GetError().c_str());

            return Audio::EAudioRequestStatus::Failure;
        }

        AudioSourceManager::Get().Initialize();

        AZ_Info(TYPEINFO_Name(), "SteamAudio system implementation initialized.");
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::ShutDown() -> Audio::EAudioRequestStatus
    {
        if (!AZ::Interface<ISoundEngine>::Get())
        {
            return Audio::EAudioRequestStatus::Failure;
        }

        AudioSourceManager::Get().Shutdown();

        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::Release() -> Audio::EAudioRequestStatus
    {
        AZLOG_INFO("SteamAudio is releasing.");
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::StopAllSounds() -> Audio::EAudioRequestStatus
    {
        AZLOG_INFO("Bop Audio received StopAllSounds.\n");

        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::RegisterAudioObject(
        Audio::IATLAudioObjectData* const audioObjectData,
        char const* const objectName) -> Audio::EAudioRequestStatus
    {
        auto* const implAudioObjectData{ static_cast<SATLAudioObjectData_steamaudio*>(
            audioObjectData) };

        implAudioObjectData->SetName(AZ::Name{ objectName });

        AZLOG(
            LOG_asi_steamaudio,
            "%s registering an audio object: [Name: %s]\n",
            TYPEINFO_Name(),
            implAudioObjectData->GetName().GetCStr());

        auto const registerOutcome{ m_engine->RegisterAudioObject(implAudioObjectData->GetId()) };

        return registerOutcome.IsSuccess() ? Audio::EAudioRequestStatus::Success
                                           : Audio::EAudioRequestStatus::Failure;
    }

    auto AudioSystemImpl_steamaudio::UnregisterAudioObject(
        Audio::IATLAudioObjectData* const audioObjectData) -> Audio::EAudioRequestStatus
    {
        auto const* const implAudioObjectData{ static_cast<SATLAudioObjectData_steamaudio const*>(
            audioObjectData) };

        AZLOG(
            LOG_asi_steamaudio,
            "%s is unregistering an audio object: [Name: %s].\n",
            TYPEINFO_Name(),
            implAudioObjectData->GetName().GetCStr());

        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::ResetAudioObject(
        Audio::IATLAudioObjectData* const audioObjectData) -> Audio::EAudioRequestStatus
    {
        auto const* const implAudioObjectData{ static_cast<SATLAudioObjectData_steamaudio const*>(
            audioObjectData) };

        AZLOG(
            LOG_asi_steamaudio,
            "%s is resetting an audio object: [Name: %s].\n",
            TYPEINFO_Name(),
            implAudioObjectData->GetName().GetCStr());

        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::UpdateAudioObject(
        Audio::IATLAudioObjectData* const audioObjectData) -> Audio::EAudioRequestStatus
    {
        AZ_UNUSED(audioObjectData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::PrepareTriggerSync(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLTriggerImplData const* const /*triggerData*/) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: PrepareTriggerSync.\n");
        AZ_UNUSED(audioObjectData);
        return Audio::EAudioRequestStatus::Success;
    }
    auto AudioSystemImpl_steamaudio::UnprepareTriggerSync(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLTriggerImplData const* const triggerData) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: UnprepareTriggerSync.\n");
        AZ_UNUSED(audioObjectData, triggerData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::PrepareTriggerAsync(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLTriggerImplData const* const triggerData,
        Audio::IATLEventData* const eventData) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: PrepareTriggerAsync.\n");
        AZ_UNUSED(audioObjectData, triggerData, eventData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::UnprepareTriggerAsync(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLTriggerImplData const* const triggerData,
        Audio::IATLEventData* const eventData) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: UnprepareTriggerAsync.\n");
        AZ_UNUSED(audioObjectData, triggerData, eventData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::ActivateTrigger(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLTriggerImplData const* const triggerData,
        Audio::IATLEventData* const eventData,
        Audio::SATLSourceData const* const /*pSourceData*/) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received ActivateTrigger");

        auto result{ Audio::EAudioRequestStatus::Failure };

        auto* implObjectData{ static_cast<SATLAudioObjectData_steamaudio*>(audioObjectData) };
        auto* implEventData{ static_cast<SATLEventData_steamaudio*>(eventData) };
        auto* const implTriggerData{ static_cast<SATLTriggerImplData_steamaudio const*>(
            triggerData) };

        if (implObjectData && implEventData && implTriggerData)
        {
            auto const objectId{ implObjectData->HasPosition() ? implObjectData->GetId()
                                                               : m_globalGameObjectId };

            auto startEventData{ StartEventData() };
            startEventData.m_gameObjectId = objectId;

            m_engine->StartEvent(startEventData);
        }

        return result;
    }

    auto AudioSystemImpl_steamaudio::StopEvent(
        Audio::IATLAudioObjectData* const /*audioObjectData*/,
        Audio::IATLEventData const* const /*eventData*/) -> Audio::EAudioRequestStatus
    {
        return Audio::EAudioRequestStatus::Failure;
    }

    auto AudioSystemImpl_steamaudio::StopAllEvents(
        Audio::IATLAudioObjectData* const audioObjectData) -> Audio::EAudioRequestStatus
    {
        AZ_Error("ASI", false, "StopAllEvents is not yet implemented.");
        AZ_UNUSED(audioObjectData);
        return Audio::EAudioRequestStatus::Failure;
    }

    auto AudioSystemImpl_steamaudio::SetPosition(
        Audio::IATLAudioObjectData* const /*audioObjectData*/,
        [[maybe_unused]] Audio::SATLWorldPosition const& worldPosition)
        -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetPosition.\n");
        return Audio::EAudioRequestStatus::Failure;
    }

    auto AudioSystemImpl_steamaudio::SetMultiplePositions(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::MultiPositionParams const& multiPositionParams) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetMultiplePositions.\n");
        AZ_UNUSED(audioObjectData, multiPositionParams);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::SetEnvironment(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLEnvironmentImplData const* const environmentData,
        float const amount) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetEnvironment.\n");
        AZ_UNUSED(audioObjectData, environmentData, amount);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::SetRtpc(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLRtpcImplData const* const rtpcData,
        float const value) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetRtpc.\n");
        AZ_UNUSED(audioObjectData, rtpcData, value);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::SetSwitchState(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLSwitchStateImplData const* const switchStateData) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetSwitchState.\n");
        AZ_UNUSED(audioObjectData, switchStateData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::SetObstructionOcclusion(
        Audio::IATLAudioObjectData* const audioObjectData,
        float const obstruction,
        float const occlusion) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: SetObstructionOcclusion.\n");
        AZ_UNUSED(audioObjectData, obstruction, occlusion);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::SetListenerPosition(
        Audio::IATLListenerData* const listenerData,
        Audio::SATLWorldPosition const& newPosition) -> Audio::EAudioRequestStatus
    {
        AZ_UNUSED(listenerData, newPosition);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::ResetRtpc(
        Audio::IATLAudioObjectData* const audioObjectData,
        Audio::IATLRtpcImplData const* const rtpcData) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: ResetRtpc.\n");
        AZ_UNUSED(audioObjectData, rtpcData);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::RegisterInMemoryFile(
        Audio::SATLAudioFileEntryInfo* const /*audioFileEntry*/) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: RegisterInMemoryFile.\n");

        return Audio::EAudioRequestStatus::Failure;
    }

    auto AudioSystemImpl_steamaudio::UnregisterInMemoryFile(
        Audio::SATLAudioFileEntryInfo* const audioFileEntry) -> Audio::EAudioRequestStatus
    {
        AZLOG(
            LOG_asi_steamaudio, "SteamAudio received a request to unregister an in-memory file.\n");
        AZ_UNUSED(audioFileEntry);
        return Audio::EAudioRequestStatus::Success;
    }

    auto AudioSystemImpl_steamaudio::ParseAudioFileEntry(
        const AZ::rapidxml::xml_node<char>* audioFileEntryNode,
        Audio::SATLAudioFileEntryInfo* const fileEntryInfo) -> Audio::EAudioRequestStatus
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request to parse an audio file entry.\n");

        static constexpr auto defaultFileEntry =
            [](Audio::SATLAudioFileEntryInfo* const entry) -> void
        {
            entry->sFileName = nullptr;
            entry->pFileData = nullptr;
            entry->pImplData = nullptr;
            entry->bLocalized = false;
            entry->nMemoryBlockAlignment = 0;
            entry->nSize = 0;
        };

        if (!audioFileEntryNode ||
            !AZ::StringFunc::Equal(audioFileEntryNode->name(), XmlTags::SteamAudioFileTag))
        {
            AZ_Error(
                "AudioSystemImpl_steamaudio",
                false,
                "Wrong node name. Expected %s, but got %s.",
                XmlTags::SteamAudioFileTag,
                audioFileEntryNode->name());

            defaultFileEntry(fileEntryInfo);

            return Audio::EAudioRequestStatus::FailureInvalidRequest;
        }

        auto const* const bopAudioFileNameAttrib =
            audioFileEntryNode->first_attribute(XmlTags::NameAttribute);

        if (!bopAudioFileNameAttrib)
        {
            AZ_Error(
                "AudioSystemImpl_steamaudio",
                false,
                "Missing attribute. Expected %s.",
                XmlTags::NameAttribute);
            defaultFileEntry(fileEntryInfo);
            return Audio::EAudioRequestStatus::FailureInvalidRequest;
        }

        auto* const soundBankFileName{ bopAudioFileNameAttrib->value() };
        auto* const implAudioFile{ azcreate(
            SATLAudioFileEntryData_steamaudio, (), Audio::AudioImplAllocator) };

        fileEntryInfo->sFileName = soundBankFileName;
        fileEntryInfo->pImplData = implAudioFile;
        fileEntryInfo->nMemoryBlockAlignment = 1;
        fileEntryInfo->bLocalized = false;

        return Audio::EAudioRequestStatus::Success;
    }

    void AudioSystemImpl_steamaudio::DeleteAudioFileEntryData(
        Audio::IATLAudioFileEntryData* const oldAudioFileEntryData)
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request to delete a file entry.\n");
        azdestroy(
            oldAudioFileEntryData, Audio::AudioImplAllocator, SATLAudioFileEntryData_steamaudio);
    }

    auto AudioSystemImpl_steamaudio::GetAudioFileLocation(
        Audio::SATLAudioFileEntryInfo* const fileEntryInfo) -> char const* const
    {
        AZLOG(
            LOG_asi_steamaudio,
            "SteamAudio received a audio file location request: [Filename: %s]\n",
            fileEntryInfo->sFileName);

        if (!fileEntryInfo)
        {
            return nullptr;
        }

        return fileEntryInfo->bLocalized ? m_localizedSoundBankFolder.c_str()
                                         : m_soundBankFolder.c_str();
    }

    auto AudioSystemImpl_steamaudio::NewAudioTriggerImplData(
        const AZ::rapidxml::xml_node<char>* audioTriggerNode) -> Audio::IATLTriggerImplData*
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request for a new audio trigger.\n");

        if (!audioTriggerNode ||
            !AZ::StringFunc::Equal(audioTriggerNode->name(), XmlTags::TriggerTag))
        {
            return nullptr;
        }

        AZStd::string const triggerName = [&audioTriggerNode]() -> decltype(triggerName)
        {
            auto const triggerNameAttrib{ audioTriggerNode->first_attribute(
                XmlTags::NameAttribute) };
            return decltype(triggerName){ triggerNameAttrib ? triggerNameAttrib->value()
                                                            : nullptr };
        }();

        auto* implAudioTriggerData{ azcreate(
            SATLTriggerImplData_steamaudio, (), Audio::AudioImplAllocator) };

        AZLOG(
            LOG_asi_steamaudio,
            "SteamAudio created a new audio trigger: [Name: %s][AtlEventId: %llu]",
            triggerName.c_str(),
            Audio::AudioStringToID<Audio::TAudioEventID>(triggerName.c_str()));

        return implAudioTriggerData;
    }

    void AudioSystemImpl_steamaudio::DeleteAudioTriggerImplData(
        Audio::IATLTriggerImplData* const oldTriggerImplData)
    {
        AZLOG(LOG_asi_steamaudio, "Steam audio received a request to delete audio trigger data.\n");
        azdestroy(oldTriggerImplData, Audio::AudioImplAllocator, SATLTriggerImplData_steamaudio);
    }

    auto AudioSystemImpl_steamaudio::NewAudioRtpcImplData(
        const AZ::rapidxml::xml_node<char>* /*audioRtpcNode*/) -> Audio::IATLRtpcImplData*
    {
        AZ_Error("ASI", false, "NewAudioRtpcImplData is not yet implemented");
        return nullptr;
    }

    void AudioSystemImpl_steamaudio::DeleteAudioRtpcImplData(
        Audio::IATLRtpcImplData* const /*oldRtpcImplData*/)
    {
        AZ_Error("ASI", false, "DeleteAudioRtpcImplData is not yet implemented");
    }

    auto AudioSystemImpl_steamaudio::NewAudioSwitchStateImplData(
        const AZ::rapidxml::xml_node<char>* /*audioSwitchStateNode*/)
        -> Audio::IATLSwitchStateImplData*
    {
        AZ_Error(
            "AudioSystemImpl_MiniAudio",
            false,
            "NewAudioSwitchStateImplData is not yet implemented.");

        return nullptr;
    }

    void AudioSystemImpl_steamaudio::DeleteAudioSwitchStateImplData(
        Audio::IATLSwitchStateImplData* const oldSwitchStateImplData)
    {
        AZLOG(
            LOG_asi_steamaudio, "SteamAudio received a request to delete audio switch state data.");

        azdestroy(oldSwitchStateImplData, Audio::AudioImplAllocator);
    }

    auto AudioSystemImpl_steamaudio::NewAudioEnvironmentImplData(
        const AZ::rapidxml::xml_node<char>* /*audioEnvironmentNode*/)
        -> Audio::IATLEnvironmentImplData*
    {
        AZLOG(
            LOG_asi_steamaudio, "SteamAudio received a request for new audio environment data.\n");

        return nullptr;
    }

    void AudioSystemImpl_steamaudio::DeleteAudioEnvironmentImplData(
        Audio::IATLEnvironmentImplData* const oldEnvironmentImplData)
    {
        azdestroy(oldEnvironmentImplData, Audio::AudioImplAllocator);
    }

    auto AudioSystemImpl_steamaudio::NewGlobalAudioObjectData(
        Audio::TAudioObjectID const /*atlObjectId*/) -> Audio::IATLAudioObjectData*
    {
        AZLOG(
            LOG_asi_steamaudio,
            "SteamAudio received a request for a new global audio object data.\n");
        return azcreate(SATLAudioObjectData_steamaudio, (), Audio::AudioImplAllocator);
    }

    auto AudioSystemImpl_steamaudio::NewAudioObjectData(Audio::TAudioObjectID const /*atlObjectId*/)
        -> Audio::IATLAudioObjectData*
    {
        return azcreate(SATLAudioObjectData_steamaudio, (), Audio::AudioImplAllocator);
    }

    void AudioSystemImpl_steamaudio::DeleteAudioObjectData(
        Audio::IATLAudioObjectData* const oldObjectData)
    {
        azdestroy(oldObjectData, Audio::AudioImplAllocator, SATLAudioObjectData_steamaudio);
    }

    auto AudioSystemImpl_steamaudio::NewDefaultAudioListenerObjectData(
        Audio::TATLIDType const objectId) -> Audio::IATLListenerData*
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: NewDefaultAudioListenerObjectData.");
        AZ_UNUSED(objectId);
        return {};
    }

    auto AudioSystemImpl_steamaudio::NewAudioListenerObjectData(Audio::TATLIDType const objectId)
        -> Audio::IATLListenerData*
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: NewAudioListenerObjectData.");
        AZ_UNUSED(objectId);
        return {};
    }

    void AudioSystemImpl_steamaudio::DeleteAudioListenerObjectData(
        Audio::IATLListenerData* const oldListenerData)
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: DeleteAudioListenerObjectData.");
        AZ_UNUSED(oldListenerData);
    }

    auto AudioSystemImpl_steamaudio::NewAudioEventData(Audio::TAudioEventID const /*eventId*/)
        -> Audio::IATLEventData*
    {
        AZLOG(LOG_asi_steamaudio, "BopAudio: NewAudioEventData");
        return azcreate(SATLEventData_steamaudio, (), Audio::AudioImplAllocator);
    }

    void AudioSystemImpl_steamaudio::DeleteAudioEventData(Audio::IATLEventData* const oldEventData)
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request to delete event data.\n");
        azdestroy(oldEventData, Audio::AudioImplAllocator);
    }

    void AudioSystemImpl_steamaudio::ResetAudioEventData(Audio::IATLEventData* const eventData)
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request to reset audio event data.\n");

        if (!eventData)
        {
            AZ_Error(
                "AudioSystemImpl_steamaudio",
                false,
                "Unable to reset audio event data because it is null.\n");
            return;
        }
    }

    auto AudioSystemImpl_steamaudio::GetImplSubPath() const -> char const* const
    {
        return SteamAudioImplSubPath;
    }

    void AudioSystemImpl_steamaudio::SetLanguage(char const* const language)
    {
        m_language = language;
    }

    auto AudioSystemImpl_steamaudio::GetImplementationNameString() const -> char const* const
    {
        return "SteamAudio-Dev";
    }

    void AudioSystemImpl_steamaudio::GetMemoryInfo(Audio::SAudioImplMemoryInfo& memoryInfo) const
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request for memory info.\n");
        AZ_UNUSED(memoryInfo);
    }

    auto AudioSystemImpl_steamaudio::GetMemoryPoolInfo()
        -> AZStd::vector<Audio::AudioImplMemoryPoolInfo>
    {
        AZLOG(LOG_asi_steamaudio, "SteamAudio received a request for memory pool info.\n");
        return {};
    }

    auto AudioSystemImpl_steamaudio::CreateAudioSource(Audio::SAudioInputConfig const& sourceConfig)
        -> bool
    {
        AudioSourceManager::Get().CreateSource(sourceConfig);

        return true;
    }

    void AudioSystemImpl_steamaudio::DestroyAudioSource(Audio::TAudioSourceId sourceId)
    {
        AudioSourceManager::Get().DestroySource(sourceId);
    }

    void AudioSystemImpl_steamaudio::SetPanningMode(Audio::PanningMode mode)
    {
        m_panningMode = mode;
    }
} // namespace SteamAudio
