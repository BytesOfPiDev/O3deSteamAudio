#pragma once

#include "AssetBuilderSDK/AssetBuilderBusses.h"

namespace SteamAudio
{
    class SaSoundAssetBuilderWorker : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        AZ_RTTI(SaSoundAssetBuilderWorker, "C79316D6-D1F0-4F7A-A2ED-F8C93B66E9E8");
        AZ_DISABLE_COPY_MOVE(SaSoundAssetBuilderWorker);

        SaSoundAssetBuilderWorker() = default;
        ~SaSoundAssetBuilderWorker() override = default;

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

    protected:
        void Copy(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;

        void BuildRaw(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;

    private:
        AZStd::atomic_bool m_isShuttingDown{};
    };
}  // namespace SteamAudio
