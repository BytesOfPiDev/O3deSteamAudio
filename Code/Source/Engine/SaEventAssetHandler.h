#pragma once

#include "Engine/SaEventAsset.h"

namespace SteamAudio
{
    class SaEventAssetHandler : public AzFramework::GenericAssetHandler<SaEventAsset>
    {
        using Base = AzFramework::GenericAssetHandler<SaEventAsset>;

    public:
        AZ_DISABLE_COPY_MOVE(SaEventAssetHandler);
        AZ_RTTI(
            SaEventAssetHandler, SaEventAssetHandlerTypeId, AzFramework::GenericAssetHandlerBase);

        SaEventAssetHandler();
        ~SaEventAssetHandler() override = default;

        void InitAsset(
            AZ::Data::Asset<AZ::Data::AssetData> const& asset,
            bool loadStageSucceeded,
            bool isReload) override;
    };
}  // namespace SteamAudio
