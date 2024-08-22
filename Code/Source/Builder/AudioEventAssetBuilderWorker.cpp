#include "Builder/AudioEventAssetBuilderWorker.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AssetBuilderSDK/SerializationDependencies.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Serialization/Utils.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "AzCore/Utils/Utils.h"
#include "AzCore/XML/rapidxml.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/Configuration.h"
#include "Engine/Parsing/AudioEventXmlParser.h"
#include "Engine/SrcSaEventAsset.h"

static constexpr auto BuildAudioEventJobKey = "Build SteamAudio event from xml";
static constexpr auto BuildEventFromEditorAssetKey = "Build SaEventAsset from editor asset";

namespace SteamAudio
{
    AudioEventAssetBuilderWorker::AudioEventAssetBuilderWorker() = default;

    void AudioEventAssetBuilderWorker::CreateJobs(
        AssetBuilderSDK::CreateJobsRequest const& request,
        AssetBuilderSDK::CreateJobsResponse& response) const
    {
        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Failed;

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

                if (sourcePath.Match(EditorSaEventAsset::ExtensionWildcard))
                {
                    descriptor.m_jobKey = BuildEventFromEditorAssetKey;
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
        /*
                auto const sourceDep = [&request]() -> AssetBuilderSDK::SourceFileDependency
                {
                    AssetBuilderSDK::SourceFileDependency result{};
                    result.m_sourceFileDependencyUUID = request.m_sourceFileUUID;
                    return result;
                }();

                response.m_sourceFileDependencyList.push_back(sourceDep);
          */
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
            BuildFromXml(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Xml build job finished.");
        }
        else if (IsRequestForJob(request, BuildEventFromEditorAssetKey))
        {
            BuildFromAsset(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Editor asset build job finished.");
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

    void AudioEventAssetBuilderWorker::BuildFromXml(
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

        AZ::Data::Asset<SaEventAsset> event{};
        event.Create(AZ::Uuid::CreateRandom());
        xmlParser.WriteIntoAsset(event.Get());

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
            absProductPath.c_str(), AZ::DataStream::ST_JSON, event.Get()) };

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

        AssetBuilderSDK::JobProduct jobProduct{};
        jobProduct.m_productFileName = absProductPath.String();
        jobProduct.m_productAssetType = SaEventAsset::TYPEINFO_Uuid();
        jobProduct.m_productSubID = SaEventAsset::AssetSubId;

        response.m_outputProducts.emplace_back(AZStd::move(jobProduct));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }

    void AudioEventAssetBuilderWorker::BuildFromAsset(
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

        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildEventFromEditorAssetKey))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Incorrect job key! Expected '%s', but got '%s'",
                BuildEventFromEditorAssetKey,
                request.m_jobDescription.m_jobKey.c_str());
            return;
        }

        if (!AZ::StringFunc::Path::IsExtension(
                request.m_sourceFile.c_str(), EditorSaEventAsset::Extension))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Source file '%s' has the wrong extension! Expected it to have '%s'.",
                request.m_sourceFile.c_str(),
                EditorSaEventAsset::Extension);
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
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
        AZ::IO::Path const absSourcePath{ request.m_fullPath };
        AZ_Info(AssetBuilderSDK::InfoWindow, "Full path: %s", absSourcePath.c_str());

        AZ::IO::Path const tempPath{ request.m_tempDirPath };
        AZ_Info(AssetBuilderSDK::InfoWindow, "Temp directory: %s", tempPath.c_str());

        AZ::IO::Path const productPath = [&absSourcePath, &request]() -> decltype(productPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= absSourcePath.Filename();
            path.ReplaceExtension(SaEventAsset::ProductExtension);
            return path;
        }();
        AZ_Info(AssetBuilderSDK::InfoWindow, "Product path: %s", productPath.c_str());

        auto const sourceAssetData = AZ::Data::Asset<EditorSaEventAsset>{
            AZ::Uuid::CreateRandom(),
            AZ::Utils::LoadObjectFromFile<EditorSaEventAsset>(absSourcePath.c_str()),
            AZ::Data::AssetLoadBehavior::PreLoad
        };

        if (!sourceAssetData)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to load source asset '%s'",
                absSourcePath.c_str());
            return;
        }

        AZ::Data::Asset<SaEventAsset> const productAssetData =
            [&sourceAssetData]() -> decltype(productAssetData)
        {
            auto result = AZ::Data::Asset<SaEventAsset>();
            result.Create(AZ::Uuid::CreateRandom());
            result->SetEventName(sourceAssetData->GetEventName());
            result->SetSound(sourceAssetData->GetSoundToPlay());
            return result;
        }();

        if (!AZ::Utils::SaveObjectToFile(
                productPath.c_str(), AZ::DataStream::ST_JSON, productAssetData.Get()))
        {
            AZ_Error(AssetBuilderSDK::ErrorWindow, false, "Failed to save %s", productPath.c_str());
            return;
        }

        AssetBuilderSDK::JobProduct const eventJobProduct =
            [&productAssetData, &productPath]() -> decltype(eventJobProduct)
        {
            auto result{ decltype(eventJobProduct){} };
            AssetBuilderSDK::OutputObject<SaEventAsset>(
                productAssetData.Get(),
                productPath.String().c_str(),
                AZ::Data::AssetType{ SaEventAsset::TYPEINFO_Uuid() },
                SaEventAsset::AssetSubId,
                result);

            return result;
        }();

        response.m_outputProducts.push_back(eventJobProduct);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}  // namespace SteamAudio
