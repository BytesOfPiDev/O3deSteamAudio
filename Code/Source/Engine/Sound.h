#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/base.h"
#include "AzCore/std/any.h"

#include "Engine/SaSoundAsset.h"

extern "C" {
struct ma_sound;
}

namespace SteamAudio
{
    class SoundInstance
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundInstance);

        SoundInstance(AZ::Data::Asset<SaSoundAsset> soundAsset);
        ~SoundInstance();

        [[nodiscard]] auto GetNative() const -> ma_sound*;

    private:
        AZStd::any m_lowLevelSound{};
        AZ::Data::Asset<AZ::Data::AssetData> m_soundAsset{};
    };

    class SoundSource
    {
    public:
        AZ_DISABLE_COPY(SoundSource);

        SoundSource() = default;
        SoundSource(AZ::Data::Asset<AZ::Data::AssetData> asset);
        SoundSource(AZ::Data::Asset<AZ::Data::AssetData> soundAsset, AZ::Name name);
        ~SoundSource();

        [[nodiscard]] auto GetName() const -> AZ::Name
        {
            return m_name;
        }

    protected:
        void Register();

    private:
        AZ::Name m_name{};
        AZ::Data::Asset<AZ::Data::AssetData> m_asset{};
    };

}  // namespace SteamAudio
