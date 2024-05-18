#include "SoundAssetBuilderWorker.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AssetBuilderSDK/SerializationDependencies.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/OpenMode.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "AzFramework/IO/LocalFileIO.h"
#include "Engine/Configuration.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    static constexpr auto BuildRawAssetKit{ "Build Raw SaSoundAsset" };
    static constexpr auto CopyRawAssetKey{ "Copy Raw SaSoundAsset" };

    void SaSoundAssetBuilderWorker::CreateJobs(
        AssetBuilderSDK::CreateJobsRequest const& request,
        AssetBuilderSDK::CreateJobsResponse& response) const
    {
        if (m_isShuttingDown)
        {
            response.m_result = AssetBuilderSDK::CreateJobsResultCode::ShuttingDown;
            return;
        }

        AZ::IO::Path const sourcePath{ request.m_sourceFile };

        auto fileIo{ AZ::IO::FileIOBase::GetInstance() };

        AZ::IO::Path const runtimeFolderPath{ fileIo->ResolvePath(BasePath).value_or("") };

        AZ_Info(
            AssetBuilderSDK::InfoWindow, "Creating job(s) for '%s'.\n", sourcePath.Native().data());

        AZStd::ranges::for_each(
            request.m_enabledPlatforms,
            [&response](auto const& info)
            {
                if (AZ::StringFunc::Equal(info.m_identifier, "server"))
                {
                    return;
                }

                AssetBuilderSDK::JobDescriptor descriptor;
                descriptor.m_jobKey = BuildRawAssetKit;
                descriptor.m_critical = true;
                descriptor.SetPlatformIdentifier(info.m_identifier.c_str());
                descriptor.m_priority = 0;

                response.m_createJobOutputs.emplace_back(descriptor);
            });

        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void SaSoundAssetBuilderWorker::ProcessJob(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        BuildRaw(request, response);
    }

    void SaSoundAssetBuilderWorker::Copy(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, CopyRawAssetKey))
        {
            return;
        }
    }

    void SaSoundAssetBuilderWorker::BuildRaw(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildRawAssetKit))
        {
            return;
        }

        if (m_isShuttingDown)
        {
            AZ_Warning(
                AssetBuilderSDK::WarningWindow,
                false,
                "Cancelling job %s due to shutdown request.",
                request.m_fullPath.c_str());

            return;
        }

        AssetBuilderSDK::JobCancelListener jobCancelListener(request.m_jobId);

        if (jobCancelListener.IsCancelled())
        {
            AZ_Error(
                AssetBuilderSDK::WarningWindow,
                false,
                "Cancel was requested for job %s.\n",
                request.m_fullPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }

        AZ::IO::Path const absSourcePath{ request.m_fullPath };

        AZ::IO::Path const absProductPath = [absSourcePath, &request]() -> decltype(absProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= request.m_sourceFile;
            path.ReplaceExtension(SaSoundAsset::ProductExtension);
            return path;
        }();

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Building asset from source file '%s'.\n",
            absSourcePath.c_str());

        AZ::IO::LocalFileIO fileIo{};
        AZ::IO::HandleType fileHandle{};

        fileIo.Open(
            absSourcePath.String().c_str(),
            AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary,
            fileHandle);

        AZ::u64 const fileSize = [&fileHandle, &fileIo]() -> decltype(fileSize)
        {
            auto result{ decltype(fileSize){} };
            fileIo.Size(fileHandle, result);
            return result;
        }();

        auto buffer = SaSoundAsset::Buffer(fileSize);
        AZ::u64 const bytesRead =
            [&fileIo, &fileHandle, &buffer, &fileSize]() -> decltype(bytesRead)
        {
            auto result{ decltype(bytesRead){} };
            fileIo.Read(fileHandle, buffer.data(), fileSize, false, &result);
            return result;
        }();

        if (bytesRead < fileSize)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Something went wrong while reading the source file. We read %u/%u bytes.\n");

            return;
        }

        fileIo.Close(fileHandle);
        fileHandle = {};

        auto const asset{ AZStd::make_unique<SaSoundAsset>(
            Audio::AudioInputSourceType::PcmFile,
            Audio::AudioInputSampleType::Float,
            AZStd::move(buffer)) };

        buffer = {};

        bool const fileSaved{ AZ::Utils::SaveObjectToFile(
            absProductPath.c_str(), AZ::DataStream::ST_BINARY, asset.get()) };

        if (!fileSaved)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to save asset to '%s'.\n",
                absProductPath.c_str());

            return;
        }

        AssetBuilderSDK::JobProduct jobProduct{};

        auto const outputSuccess{ AssetBuilderSDK::OutputObject(
            asset.get(),
            absProductPath.String(),
            azrtti_typeid<SaSoundAsset>(),
            SaSoundAsset::AssetSubId,
            jobProduct) };

        if (!outputSuccess)
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to output object.");
            return;
        }

        response.m_outputProducts.emplace_back(AZStd::move(jobProduct));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}  // namespace SteamAudio
