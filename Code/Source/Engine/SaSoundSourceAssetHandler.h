#pragma once

#include "AzFramework/Asset/GenericAssetHandler.h"
#include "Engine/SaSoundSourceAsset.h"
namespace SteamAudio
{
    class SaSoundSourceAssetHandler : public AzFramework::GenericAssetHandler<SaSoundSourceData>
    {
        using Base = AzFramework::GenericAssetHandler<SaSoundSourceData>;

    public:
        AZ_DISABLE_COPY_MOVE(SaSoundSourceAssetHandler);
        AZ_RTTI_WITH_NAME(
            SaSoundSourceAssetHandler,
            "SaSoundSourceAssetHandler",
            "{49CA5860-3710-43EC-8D6F-464B5B87D0B9}",
            GenericAssetHandlerBase);

        SaSoundSourceAssetHandler();
        ~SaSoundSourceAssetHandler() override = default;

        void InitAsset(
            const AZ::Data::Asset<AZ::Data::AssetData>& asset,
            bool loadStageSucceeded,
            bool isReload) override;
    };
}  // namespace SteamAudio
