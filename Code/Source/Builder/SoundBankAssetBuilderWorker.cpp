#include "Builder/SoundBankAssetBuilderWorker.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AzCore/Asset/AssetDataStream.h"
#include "AzCore/StringFunc/StringFunc.h"

static constexpr auto BuildSoundBankJobKey = "Build Steam SoundBank";
namespace SteamAudio
{

    void SoundBankAssetBuilderWorker::CreateJobs(
        AssetBuilderSDK::CreateJobsRequest const& request,
        AssetBuilderSDK::CreateJobsResponse& response) const
    {
        AZ_Info(TYPEINFO_Name(), "Creating jobs ...");
        if (m_isShuttingDown)
        {
            response.m_result = AssetBuilderSDK::CreateJobsResultCode::ShuttingDown;
            return;
        }

        AZStd::ranges::for_each(
            request.m_enabledPlatforms,
            [&response](auto const& info)
            {
                if (info.m_identifier == "server")
                {
                    return;
                }

                AssetBuilderSDK::JobDescriptor descriptor;
                descriptor.m_jobKey = BuildSoundBankJobKey;
                descriptor.m_critical = true;
                descriptor.SetPlatformIdentifier(info.m_identifier.c_str());
                descriptor.m_priority = 0;
                response.m_createJobOutputs.push_back(descriptor);
            });

        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void SoundBankAssetBuilderWorker::ProcessJob(
        [[maybe_unused]] AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        if (m_isShuttingDown)
        {
            AZ_Warning(
                AssetBuilderSDK::WarningWindow,
                false,
                "Cancelling job %s due to shutdown request.",
                request.m_fullPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        // Verify job key is correct
        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildSoundBankJobKey))
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Job failed. Unsupported job key: '%s'");

            return;
        }

        AssetBuilderSDK::JobCancelListener jobCancelListener(request.m_jobId);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
    }

} // namespace SteamAudio
