#pragma once

#include "Engine/SaEventAsset.h"

namespace SteamAudio
{
    struct SaEventAssetHandler : public SaEventAssetGenericHandler
    {
        AZ_RTTI(
            SaEventAssetHandler, SaEventAssetHandlerTypeId, AzFramework::GenericAssetHandlerBase);

        void InitAsset(
            AZ::Data::Asset<AZ::Data::AssetData> const& asset,
            bool loadStageSucceeded,
            bool isReload) override;
    };
}  // namespace SteamAudio
