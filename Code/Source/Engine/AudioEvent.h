#pragma once

#include "Engine/AudioObject.h"
#include "Engine/Id.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    using SaAudioEventState = Audio::EAudioEventState;

    class SaEvent
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SaEvent);

        using StartFunc = AZStd::function<void()>;
        using StopFunc = AZStd::function<void()>;

        SaEvent() = default;
        SaEvent(StartFunc startFunc, StopFunc stopFunc)
            : m_startFunc{ AZStd::move(startFunc) }
            , m_stopFunc{ AZStd::move(stopFunc) } {};

        ~SaEvent() = default;

        void Start(SaGameObjectId parentObject = {});
        void Stop(SaGameObjectId parentObject = {});
        void Update(float);

        [[nodiscard]] auto GetEventState() const -> SaAudioEventState
        {
            return m_eventState;
        }

    private:
        AZ::Data::Asset<SaSoundAsset> m_soundAsset{};
        StartFunc m_startFunc{};
        StopFunc m_stopFunc{};

        SaAudioEventState m_eventState{};
    };
}  // namespace SteamAudio
