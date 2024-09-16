#include "Engine/SaEventAssetHandler.h"

#include "Engine/SaEventAsset.h"
#include "Engine/Tasks/Task.h"

namespace SteamAudio
{
    void SaEventAssetHandler::InitAsset(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset, bool loadStageSucceeded, bool isReload)
    {
        auto* const eventAsset{ asset.GetAs<SaEventAsset>() };

        AZStd::ranges::for_each(
            eventAsset->GetTasksDefinitions(),
            [](TaskDefinition taskDef)
            {
                taskDef.m_config.m_asset.QueueLoad();
                taskDef.m_config.m_asset.BlockUntilLoadComplete();
            });

        SaEventAssetGenericHandler::InitAsset(asset, loadStageSucceeded, isReload);
    };
}  // namespace SteamAudio
