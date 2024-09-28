#include "Engine/SaEventAssetHandler.h"

#include "Engine/SaEventAsset.h"

namespace SteamAudio
{
    SaEventAssetHandler::SaEventAssetHandler()
        : Base("SteamAudio Event", "Sound", SaEventAsset::Extension)
    {
    }

    void SaEventAssetHandler::InitAsset(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset, bool loadStageSucceeded, bool isReload)
    {
        if (loadStageSucceeded)
        {
            auto* const eventAsset{ asset.GetAs<SaEventAsset>() };

            AZStd::ranges::for_each(
                eventAsset->GetSoundSourceAssets(),
                [](auto& soundSourceAsset)
                {
                    // soundSourceAsset->LoadSoundSourceDeps();
                });
        }

        Base::InitAsset(asset, loadStageSucceeded, isReload);
    };
}  // namespace SteamAudio
