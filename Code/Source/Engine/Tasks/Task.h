#pragma once

#include "AzCore/Asset/AssetCommon.h"

#include "Engine/SoundConfig.h"

namespace SteamAudio
{
    struct TaskDefinition
    {
        AZ_TYPE_INFO_WITH_NAME_DECL(TaskDefinition);

        static void Reflect(AZ::ReflectContext* context);

        [[nodiscard]] auto GetConfig() const -> SoundTaskConfig const&
        {
            return m_config;
        }

        //! The configuration file that describes to setup the task associated with our asset
        SoundTaskConfig m_config;
    };
}  // namespace SteamAudio
