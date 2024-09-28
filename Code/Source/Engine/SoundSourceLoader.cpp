#include "Engine/SoundSourceLoader.h"

#include "AudioFileUtils.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/Path/Path_fwd.h"
#include "Engine/Configuration.h"
#include "Engine/SaSoundSource.h"
#include "Engine/SaSoundSourceAsset.h"

namespace SteamAudio
{
    void SoundSourceLoader::Load()
    {
        auto const soundSourceFiles{ Audio::FindFilesInPath(
            BanksAlias, SaSoundSourceData::ExtensionWildcard) };

        AZStd::ranges::for_each(
            soundSourceFiles,
            [this](AZ::IO::FixedMaxPath const& aliasedPath)
            {
                AZ::IO::FixedMaxPath replacedPath{};
                AZ::IO::FileIOBase::GetInstance()->ReplaceAlias(replacedPath, aliasedPath);

                AZ::Data::AssetId const soundSourceAssetId =
                    [&replacedPath]() -> decltype(soundSourceAssetId)
                {
                    auto result{ decltype(soundSourceAssetId){} };
                    AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                        result,
                        &AZ::Data::AssetCatalogRequests::GetAssetIdByPath,
                        replacedPath.c_str(),
                        azrtti_typeid<SaSoundSourceData>(),
                        false);

                    return result;
                }();

                if (!soundSourceAssetId.IsValid())
                {
                    AZLOG_ERROR(
                        "Failed to find SoundSource asset for '%s' - no asset id found",
                        replacedPath.c_str());
                    return;
                }

                auto const& soundSrc{ m_sources.emplace_back(AZStd::make_unique<SoundSource>(
                    AZ::Data::AssetManager::Instance().GetAsset<SaSoundSourceData>(
                        soundSourceAssetId, AZ::Data::AssetLoadBehavior::Default))) };

                AZLOG_INFO(
                    "Found SoundSource asset '%s' w/ id '%s' and named '%s'",
                    replacedPath.c_str(),
                    soundSourceAssetId.ToFixedString().c_str(),
                    soundSrc->GetName().GetCStr());
            });
    }
}  // namespace SteamAudio
