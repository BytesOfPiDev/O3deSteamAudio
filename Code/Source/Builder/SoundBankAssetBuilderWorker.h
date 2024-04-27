#pragma once

#include <AssetBuilderSDK/AssetBuilderBusses.h>

namespace SteamAudio 
{
    class SoundBankAssetBuilderWorker : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        AZ_RTTI(SoundBankAssetBuilderWorker, "{E0DBCA98-39A8-4069-B773-844A16691CA8}");

        SoundBankAssetBuilderWorker() = default;

        void CreateJobs(
            AssetBuilderSDK::CreateJobsRequest const& request,
            AssetBuilderSDK::CreateJobsResponse& response) const;

        void ProcessJob(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;

        void ShutDown() override
        {
            m_isShuttingDown = true;
        }

    private:
        AZStd::atomic_bool m_isShuttingDown{};
    };

} // namespace BopAudio
