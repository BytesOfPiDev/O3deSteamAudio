#pragma once

#include "AssetBuilderSDK/AssetBuilderBusses.h"

namespace  SteamAudio
{
    class AudioEventAssetBuilderWorker : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        AZ_RTTI(AudioEventAssetBuilderWorker, "{1BBC3FEF-229C-4C56-A0F9-134948EB50BB}");

        AudioEventAssetBuilderWorker();

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
        void CreateFactories();

        void Copy(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;
        void Build(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;

    private:
        AZStd::atomic_bool m_isShuttingDown{};
    };
} // namespace BopAudio
