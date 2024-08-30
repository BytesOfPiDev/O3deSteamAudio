#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

#include "Engine/SaSoundAsset.h"

namespace SteamAudio
{
    struct SoundTaskConfig
    {
        AZ_TYPE_INFO(SoundTaskConfig, "FC1ECB5C-C057-4B87-BEEE-97DC538F01AA");

        static void Reflect(AZ::ReflectContext* context);

        static constexpr auto VolumeMax = 1.0f;
        static constexpr auto VolumeMin = 0.0f;
        static constexpr auto VolumeStep = 0.1f;

        AZ::Data::Asset<SaSoundAsset> m_asset{};
        float m_volume;
        bool m_loop;
    };
}  // namespace SteamAudio
