#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    struct SoundConfig
    {
        AZ_TYPE_INFO(SoundConfig, SoundConfigDataTypeId);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr auto VolumeMax = 1.0f;
        static constexpr auto VolumeMin = 0.0f;
        static constexpr auto VolumeStep = 0.1f;

        float m_volume{ 1.0f };
        bool m_loop{ false };
    };
}  // namespace SteamAudio
