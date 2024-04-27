#pragma once

#include "AzCore/Math/Uuid.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    using SaUniqueId = AZ::Uuid;

    using SaInstanceId = AZ::u32;
    static constexpr auto InvalidInstanceId{ 0 };

    using SaGameObjectId = AZ::EntityId;
    using SaId = Audio::TATLIDType;
} // namespace SteamAudio
