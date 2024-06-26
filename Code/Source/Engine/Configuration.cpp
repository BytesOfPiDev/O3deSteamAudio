#include "Engine/Configuration.h"

#include "AzCore/IO/Path/Path.h"

namespace SteamAudio
{
    constexpr auto GetBanksRootPath() -> AZ::IO::PathView
    {
        return BanksAlias;
    }
} // namespace SteamAudio