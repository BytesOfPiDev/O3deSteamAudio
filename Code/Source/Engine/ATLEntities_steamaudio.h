#pragma once

#include "ATLEntityData.h"

#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    class SATLAudioObjectData_steamaudio : public Audio::IATLAudioObjectData
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SATLAudioObjectData_steamaudio);

        SATLAudioObjectData_steamaudio() = default;
        ~SATLAudioObjectData_steamaudio() override = default;
    };

    struct SATLTriggerImplData_steamaudio : public Audio::IATLTriggerImplData
    {
        SATLTriggerImplData_steamaudio() = default;
        ~SATLTriggerImplData_steamaudio() override = default;
    };

    struct SATLListenerData_BopAudio : public Audio::IATLListenerData
    {
        explicit SATLListenerData_BopAudio(Audio::TAudioObjectID id)
            : m_listenerObjectId(id)
        {
        }

        Audio::TAudioObjectID m_listenerObjectId{};
    };

    class SATLEventData_steamaudio : public Audio::IATLEventData
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SATLEventData_steamaudio);

        SATLEventData_steamaudio() = default;
        ~SATLEventData_steamaudio() override = default;
    };

    struct SATLAudioFileEntryData_steamaudio : public Audio::IATLAudioFileEntryData
    {
        AZ_DEFAULT_COPY_MOVE(SATLAudioFileEntryData_steamaudio);

        SATLAudioFileEntryData_steamaudio() = default;
        ~SATLAudioFileEntryData_steamaudio() override = default;
    };

} // namespace SteamAudio