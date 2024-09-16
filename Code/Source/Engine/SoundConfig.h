#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

#include "Engine/SaSoundAsset.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    struct SoundTaskConfig
    {
        AZ_TYPE_INFO(SoundTaskConfig, SoundTaskConfigTypeId);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr auto VolumeMax = 1.0f;
        static constexpr auto VolumeMin = 0.0f;
        static constexpr auto VolumeStep = 0.1f;

        void AssetChanged();

        AZ::Data::Asset<SaSoundAsset> m_asset{ AZ::Data::AssetLoadBehavior::PreLoad };
        float m_volume{ 1.0f };
        bool m_loop{ false };
    };
}  // namespace SteamAudio
