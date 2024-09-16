#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/base.h"
#include "AzCore/std/any.h"

#include "Engine/SaSoundAsset.h"
#include "Engine/SoundConfig.h"

extern "C" {
struct ma_sound;
}

namespace SteamAudio
{
    class SoundInstance
    {
    public:
        SoundInstance(AZ::Name soundName, bool loop = false, float volume = 1.0f);
        SoundInstance(SoundInstance const&) = delete;
        SoundInstance(SoundInstance&& other);
        auto operator=(SoundInstance const&) -> SoundInstance& = delete;
        auto operator=(SoundInstance&& other) -> SoundInstance& = delete;
        ~SoundInstance();

        [[nodiscard]] auto GetNative() const -> ma_sound*;

    private:
        AZStd::any m_lowLevelSound{};
        AZ::Name m_soundName{};
    };

    class SoundSource
    {
    public:
        AZ_DISABLE_COPY(SoundSource);

        SoundSource() = default;
        SoundSource(AZ::Data::Asset<SaSoundAsset> soundAsset, AZ::Name name);
        ~SoundSource();

        [[nodiscard]] auto GetName() const -> AZ::Name
        {
            return m_name;
        }

    protected:
        void Register();

    private:
        AZ::Name m_name{};
        AZ::Data::Asset<SaSoundAsset> m_asset{};
    };

}  // namespace SteamAudio
