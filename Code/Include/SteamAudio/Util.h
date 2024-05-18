#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/IO/Path/Path.h"

namespace SteamAudio::Util
{
    inline auto GetAssetPath(AZ::Data::AssetId const& id) -> AZ::IO::Path
    {
        return [&id]() -> decltype(GetAssetPath(id))
        {
            auto result{ decltype(GetAssetPath(id)){} };
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                result, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetPathById, id);

            return result;
        }();
    }

    class Defer
    {
        using DeferFunc = AZStd::function<void(void)>;

    public:
        AZ_DISABLE_COPY_MOVE(Defer);
        Defer(DeferFunc&& func)
            : m_func(func)
        {
        }

        Defer() = delete;
        ~Defer()
        {
            m_func();
        }

    private:
        DeferFunc m_func;
    };
}  // namespace SteamAudio::Util
