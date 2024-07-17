#include "SoundAssetBuilderWorker.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AssetBuilderSDK/SerializationDependencies.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Configuration.h"
#include "Engine/SoundAsset.h"
#include "SteamAudio/Util.h"
#include "SteamAudio/dr_wav.h"

namespace SteamAudio
{
    static constexpr auto BuildRawSourceKey{ "Build Raw (SteamAudio)" };
    static constexpr auto CopyExistingAssetKey{ "Copy Raw SaSoundAsset" };
    static constexpr auto BuildWavSourceKey{ "Build Wav (SteamAudio)" };

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
            [&response, &request](auto const& info)
            {
                if (AZ::StringFunc::Equal(info.m_identifier, "server"))
                {
                    return;
                }

                AZ::IO::Path const sourcePath{ request.m_sourceFile.c_str() };

                AssetBuilderSDK::JobDescriptor descriptor;
                if (sourcePath.Match(SaSoundAsset::RawExtensionWildcard))
                {
                    descriptor.m_jobKey = BuildRawSourceKey;
                }
                else if (sourcePath.Match(SaSoundAsset::ProductExtensionWildcard))
                {
                    descriptor.m_jobKey = CopyExistingAssetKey;
                }
                else if (sourcePath.Match(SaSoundAsset::WavExtensionWildcard))
                {
                    descriptor.m_jobKey = BuildWavSourceKey;
                }
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
        if (AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildRawSourceKey))
        {
            BuildRawSource(request, response);
        }
        else if (AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, CopyExistingAssetKey))
        {
            CopyExistingAsset(request, response);
        }
        else if (AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildWavSourceKey))
        {
            BuildWavSourceDecoded(request, response);
        }
        else
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Unhandled job key: %s",
                request.m_jobDescription.m_jobKey.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
        }
    }

    void SaSoundAssetBuilderWorker::CopyExistingAsset(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, CopyExistingAssetKey))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Job key '%s' is not equal to %s",
                request.m_jobDescription.m_jobKey.c_str(),
                CopyExistingAssetKey,
                request.m_jobDescription.m_jobKey.c_str());

            return;
        }

        AZ::IO::Path const absSourcePath{ request.m_fullPath };

        AZ::IO::Path const absProductPath = [absSourcePath, &request]() -> decltype(absProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= request.m_sourceFile;
            return path;
        }();

        AZ_Info(AssetBuilderSDK::InfoWindow, "Copying...", absSourcePath.c_str());

        auto assetData{ AZStd::unique_ptr<SaSoundAsset>{
            AZ::Utils::LoadObjectFromFile<SaSoundAsset>(request.m_fullPath) } };

        AssetBuilderSDK::JobProduct jobProduct{};

        auto const outputSuccess{ AssetBuilderSDK::OutputObject(
            assetData.get(),
            absProductPath.String(),
            azrtti_typeid<SaSoundAsset>(),
            SaSoundAsset::AssetSubId,
            jobProduct) };

        if (!outputSuccess)
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Copy failed.");
            return;
        }

        AZ_Info(AssetBuilderSDK::InfoWindow, "Copy successful.", absSourcePath.c_str());

        response.m_outputProducts.emplace_back(AZStd::move(jobProduct));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }

    void SaSoundAssetBuilderWorker::BuildRawSource(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildRawSourceKey))
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

        auto const fileBuffer{ Util::LoadFileIntoBuffer(request.m_fullPath) };
        auto const* const beginRaw = [&fileBuffer]() -> AZ::u8 const*
        {
            auto* begin{ static_cast<void const*>(fileBuffer.begin()) };
            return static_cast<AZ::u8 const*>(begin);
        }();

        auto rawOutputBuffer{ AZStd::span<AZ::u8 const>(
            static_cast<AZ::u8 const*>(beginRaw), fileBuffer.size()) };

        auto const asset{ AZStd::make_unique<SaSoundAsset>(
            Audio::AudioInputSourceType::PcmFile,
            Audio::AudioInputSampleType::Float,
            DefaultAudioChannels,
            DefaultSampleRate,
            rawOutputBuffer) };

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

    void SaSoundAssetBuilderWorker::BuildWavSource(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildWavSourceKey))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Incorrect job key: %s. Expected: %s.",
                request.m_jobDescription.m_jobKey.c_str(),
                BuildWavSourceKey);
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

        AZ::IO::Path const absProductPath = [&request]() -> decltype(absProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= request.m_sourceFile;
            path.ReplaceExtension(SaSoundAsset::ProductExtension);
            return path;
        }();

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Building SteamAudio wav asset from source file '%s'.\n",
            request.m_fullPath.c_str());

        auto fileBuffer = Util::LoadFileIntoBuffer(request.m_fullPath);

        if (fileBuffer.empty())
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Empty buffer when loading wav file");
            return;
        }

        AZ_Info(AssetBuilderSDK::InfoWindow, "Buffer size: %llu bytes", fileBuffer.size());

        AZ::Data::Asset<SaSoundAsset> asset;
        asset.Create(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));

        void const* const beg = [&fileBuffer]() -> decltype(beg)
        {
            return static_cast<AZ::u8*>(static_cast<void*>(fileBuffer.begin()));
        }();

        auto const wavOutput{ AZStd::span<AZ::u8 const>(
            static_cast<AZ::u8 const*>(beg), fileBuffer.size()) };

        asset->SetAudioData(Audio::AudioInputSourceType::WavFile, wavOutput);

        if (!AZ::Utils::SaveObjectToFile(
                absProductPath.c_str(), AZ::DataStream::ST_BINARY, asset.Get()))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow, false, "Failed to save %s", absProductPath.c_str());
            return;
        }

        AssetBuilderSDK::JobProduct jobProduct;
        if (!AssetBuilderSDK::OutputObject(
                asset.Get(),
                absProductPath.c_str(),
                azrtti_typeid<SaSoundAsset>(),
                SaSoundAsset::AssetSubId,
                jobProduct))
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to output product dependencies.");
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
        }
        else
        {
            response.m_outputProducts.push_back(AZStd::move(jobProduct));
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
        }
    }

    void SaSoundAssetBuilderWorker::BuildWavSourceDecoded(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildWavSourceKey))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Incorrect job key: %s. Expected: %s.",
                request.m_jobDescription.m_jobKey.c_str(),
                BuildWavSourceKey);
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
            "Building SteamAudio wav asset from source file '%s'.\n",
            absSourcePath.c_str());

        auto resolvePathResult =
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(request.m_fullPath.c_str());

        drwav wav;

        bool initWavFileSuccess{ static_cast<bool>(
            drwav_init_file(&wav, resolvePathResult->c_str(), nullptr)) };

        if (!initWavFileSuccess)
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to init the wav file.");
            return;
        }

        Util::Defer const cleanupWav{ [&wav]()
                                      {
                                          drwav_uninit(&wav);
                                      } };

        AZ_Info(AssetBuilderSDK::InfoWindow, "PCM Frame Count: %llu", wav.totalPCMFrameCount);
        AZ_Info(AssetBuilderSDK::InfoWindow, "Channels: %llu", wav.channels);
        AZ_Info(AssetBuilderSDK::InfoWindow, "SampleRate: %llu", wav.sampleRate);

        auto interleavedData{ SaSoundAsset::Buffer(
            wav.totalPCMFrameCount * wav.channels * sizeof(float)) };

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Interleaved data size : %llu bytes",
            interleavedData.size());

        auto const framesRead{ drwav_read_pcm_frames_f32(
            &wav,
            wav.totalPCMFrameCount,
            static_cast<float*>(static_cast<void*>(interleavedData.data()))) };

        if (framesRead != wav.totalPCMFrameCount)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to read all frames: %llu / %llu frames.",
                framesRead,
                interleavedData.size());

            return;
        }

        auto const asset{ AZStd::make_unique<SaSoundAsset>(
            Audio::AudioInputSourceType::PcmFile,
            Audio::AudioInputSampleType::Float,
            static_cast<AZ::u32>(wav.channels),
            static_cast<AZ::u32>(wav.sampleRate),
            interleavedData) };

        interleavedData.clear();

        bool const fileSaved{ AZ::Utils::SaveObjectToFile(
            absProductPath.c_str(), AZ::DataStream::ST_BINARY, asset.get()) };

        if (!fileSaved)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to save asset to '%s'.",
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

        response.m_outputProducts.emplace_back(jobProduct);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}  // namespace SteamAudio
