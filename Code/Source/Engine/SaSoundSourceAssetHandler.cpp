#include "Engine/SaSoundSourceAssetHandler.h"
#include "Engine/SaSoundSourceAsset.h"

namespace SteamAudio
{
    SaSoundSourceAssetHandler::SaSoundSourceAssetHandler()
        : Base{ "SteamAudio Source", "Sound", SaSoundSourceData::Extension } {};

    void SaSoundSourceAssetHandler::InitAsset(
        const AZ::Data::Asset<AZ::Data::AssetData>& asset, bool loadStageSucceeded, bool isReload)
    {
        if (loadStageSucceeded)
        {
            auto* const ptr{ asset.GetAs<SaSoundSourceData>() };
            ptr->LoadSoundSourceDeps();
        }

        Base::InitAsset(asset, loadStageSucceeded, isReload);
    }
}  // namespace SteamAudio
