#include "Builder/AudioEventAssetBuilderWorker.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AssetBuilderSDK/SerializationDependencies.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "AzCore/Utils/Utils.h"
#include "AzCore/XML/rapidxml.h"
#include "Engine/Configuration.h"
#include "Engine/Parsing/AudioEventXmlParser.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

#include "Engine/AudioEventAsset.h"

static constexpr auto BuildAudioEventJobKey = "Build SteamAudio Event";
static constexpr auto CopyAudioEventJobKey = "Copy SteamAudio Event";

namespace SteamAudio
{
    AudioEventAssetBuilderWorker::AudioEventAssetBuilderWorker() = default;

    void AudioEventAssetBuilderWorker::CreateJobs(
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
            [&response, &sourcePath](auto const& info)
            {
                if (AZ::StringFunc::Equal(info.m_identifier, "server"))
                {
                    return;
                }

                AssetBuilderSDK::JobDescriptor descriptor;

                if (sourcePath.Match(SaEventAsset::ProductExtensionWildcard))
                {
                    descriptor.m_jobKey = CopyAudioEventJobKey;
                }
                else if (sourcePath.Match(SaEventAsset::SourceExtensionWildcard))
                {
                    descriptor.m_jobKey = BuildAudioEventJobKey;
                }
                else
                {
                    AZ_Error(
                        TYPEINFO_Name(),
                        false,
                        "Unsupported file type: %s.\n",
                        sourcePath.Extension().String().c_str());

                    return;
                }

                descriptor.m_critical = true;
                descriptor.SetPlatformIdentifier(info.m_identifier.c_str());
                descriptor.m_priority = 0;

                response.m_createJobOutputs.emplace_back(descriptor);
            });

        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void AudioEventAssetBuilderWorker::ProcessJob(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

        static constexpr auto IsRequestForJob = [](auto const& request, auto const& key) -> bool
        {
            return AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, key);
        };

        AZ_Info(AssetBuilderSDK::InfoWindow, "Processing Job Key: '%s'.\n", BuildAudioEventJobKey);

        if (IsRequestForJob(request, BuildAudioEventJobKey))
        {
            Build(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Build job finished.");
        }
        else if (IsRequestForJob(request, CopyAudioEventJobKey))
        {
            Copy(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Copy job finished.");
        }
        else
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Job failed. Unsupported job key: '%s'",
                request.m_jobDescription.m_jobKey.c_str());

            return;
        }
    }
    void AudioEventAssetBuilderWorker::Build(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
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

        AZ::IO::Path const absSourcePath = [&request]() -> decltype(absSourcePath)
        {
            auto path = decltype(absSourcePath){ AZ::Utils::GetProjectPath() };
            path /= request.m_sourceFile;

            return path;
        }();

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Building asset from source file '%s'.\n",
            absSourcePath.c_str());

        AZStd::unique_ptr<AZ::rapidxml::xml_document<char>> docPtr =
            [&absSourcePath]() -> decltype(docPtr)
        {
            auto resultPtr{ AZStd::make_unique<decltype(docPtr)::element_type>() };
            AZStd::string pathToXmlFile{ absSourcePath.c_str() };

            AZStd::string buffer{ AZ::Utils::ReadFile(absSourcePath.c_str()).GetValueOr("") };

            resultPtr->parse<0>(buffer.data());

            return AZStd::move(resultPtr);
        }();

        if (docPtr->isError())
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to parse xml file '%s'due to error '%s'.\n",
                absSourcePath.c_str(),
                docPtr->getError());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AZ_Info(AssetBuilderSDK::InfoWindow, "Successfully parsed xml file.\n");

        AudioEventXmlParser xmlParser{ AZStd::move(docPtr) };
        docPtr = nullptr;

        auto event{ AZStd::make_unique<SaEventAsset>() };
        xmlParser.WriteIntoAsset(event.get());

        AZ::IO::Path const absProductPath =
            [absSourcePath, &request, &event]() -> decltype(absProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= request.m_sourceFile;
            path.ReplaceFilename(event->GetEventName().c_str());
            path.ReplaceExtension(SaEventAsset::ProductExtension);
            return path;
        }();

        event->SetEventName(absProductPath.Filename().Stem().String());

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Saving asset. Name: %s | Id: %zu",
            event->GetEventName().c_str(),
            event->GetEventId());

        bool const fileSaved{ AZ::Utils::SaveObjectToFile(
            absProductPath.c_str(), AZ::DataStream::ST_JSON, event.get()) };

        if (!fileSaved)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to save asset to '%s'.\n",
                absProductPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AssetBuilderSDK::JobProduct jobProduct{ absProductPath.String() };
        jobProduct.m_outputFlags = AssetBuilderSDK::ProductOutputFlags::ProductAsset;
        jobProduct.m_dependenciesHandled = true;
        jobProduct.m_productFileName = absProductPath.String();

        bool const outputSuccess = AssetBuilderSDK::OutputObject(
            event.get(),
            absProductPath.c_str(),
            AZ::Data::AssetType{ SaEventAsset::TYPEINFO_Uuid() },
            SaEventAsset::AssetSubId,
            jobProduct);

        if (!outputSuccess)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to output job product for '%s'\n",
                absProductPath.Native().data());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        response.m_outputProducts.emplace_back(AZStd::move(jobProduct));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }

    void AudioEventAssetBuilderWorker::Copy(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        if (m_isShuttingDown)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Cancelling job %s due to shutdown request.",
                request.m_fullPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }

        AssetBuilderSDK::JobCancelListener jobCancelListener(request.m_jobId);

        if (jobCancelListener.IsCancelled())
        {
            AZ_Warning(
                AssetBuilderSDK::WarningWindow,
                false,
                "Cancel was requested for job %s.\n",
                request.m_fullPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }

        if (!AZ::StringFunc::Path::IsExtension(
                request.m_sourceFile.c_str(), SaEventAsset::ProductExtension))
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Source file has the wrong extension!");
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AZ::IO::Path const sourceFilename = [&request]() -> decltype(sourceFilename)
        {
            AZStd::string filename{};
            AZ::StringFunc::Path::GetFileName(request.m_sourceFile.c_str(), filename);

            return filename;
        }();

        AZ::IO::Path const absProductPath = [&request]() -> decltype(absProductPath)
        {
            AZStd::string path{};
            AZ::StringFunc::Path::Join(
                request.m_tempDirPath.c_str(), request.m_sourceFile.c_str(), path);

            AZ_Assert(
                AZ::StringFunc::Path::IsExtension(path.c_str(), SaEventAsset::ProductExtension),
                "Product file has the wrong extension!");

            return path;
        }();

        auto asset = AZStd::unique_ptr<SaEventAsset>{ AZ::Utils::LoadObjectFromFile<SaEventAsset>(
            request.m_fullPath) };
        if (asset == nullptr)
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to load asset.");

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        auto const successfulCopy{ AZ::IO::FileIOBase::GetInstance()->Copy(
            request.m_fullPath.c_str(), absProductPath.c_str()) };

        if (!successfulCopy)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to copy pre-existing product asset from [%s] to [%s].",
                request.m_fullPath.c_str(),
                absProductPath.c_str());

            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AssetBuilderSDK::JobProduct jobProduct{};

        auto const outputSuccess{ AssetBuilderSDK::OutputObject(
            asset.get(),
            absProductPath.String(),
            azrtti_typeid<SaEventAsset>(),
            SaEventAsset::AssetSubId,
            jobProduct) };

        if (!outputSuccess)
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to output object.");
            return;
        }

        response.m_outputProducts.push_back(jobProduct);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}  // namespace SteamAudio
