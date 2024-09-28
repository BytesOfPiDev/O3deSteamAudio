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

#include "Engine/Configuration.h"
#include "Engine/Parsing/AudioEventXmlParser.h"
#include "Engine/SaEventAsset.h"

static constexpr auto BuildAudioEventJobKey = "Build SteamAudio event from xml";

namespace SteamAudio
{
    AudioEventAssetBuilderWorker::AudioEventAssetBuilderWorker() = default;

    auto AudioEventAssetBuilderWorker::CanBuildFromAsset(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse const&) const -> bool
    {
        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, BuildAudioEventJobKey))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Incorrect job key! Expected '%s', but got '%s'",
                BuildAudioEventJobKey,
                request.m_jobDescription.m_jobKey.c_str());
            return false;
        }

        if (!AZ::StringFunc::Path::IsExtension(
                request.m_sourceFile.c_str(), SaEventAsset::Extension))
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Source file '%s' has the wrong extension! Expected it to have '%s'.",
                request.m_sourceFile.c_str(),
                SaEventAsset::Extension);
            return false;
        }

        return true;
    }

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

                if (sourcePath.Match(SaEventAsset::ExtensionWildcard))
                {
                    descriptor.m_jobKey = BuildAudioEventJobKey;
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

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Processing Job Key: '%s'.\n",
            request.m_jobDescription.m_jobKey.c_str());

        if (AZ::StringFunc::Path::IsExtension(request.m_sourceFile.c_str(), "xml"))
        {
            BuildFromXml(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Finished building from xml.");
        }
        else if (AZ::StringFunc::Path::IsExtension(
                     request.m_sourceFile.c_str(), SaEventAsset::Extension))
        {
            BuildFromAsset(request, response);
            AZ_Info(AssetBuilderSDK::InfoWindow, "Finished building from source event asset.");
        }
        else
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Source file has unsupported extension: '%s'",
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

        AZ::Data::Asset<SaEventAsset> eventAsset{ AZ::Data::AssetLoadBehavior::PreLoad };
        eventAsset.Create(AZ::Uuid::CreateRandom());
        xmlParser.WriteIntoAsset(eventAsset.Get());

        AZ::IO::Path const absProductPath =
            [absSourcePath, &request, &eventAsset]() -> decltype(absProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= request.m_sourceFile;
            path.ReplaceFilename(eventAsset->GetEventName().c_str());
            path.ReplaceExtension(SaEventAsset::Extension);
            return path;
        }();

        eventAsset->SetEventName(absProductPath.Filename().Stem().String());

        AZ_Info(
            AssetBuilderSDK::InfoWindow,
            "Saving asset w/ event name '%s', id '%zu'",
            eventAsset->GetEventName().c_str(),
            eventAsset->GetEventId());

        bool const fileSaved{ AZ::Utils::SaveObjectToFile(
            absProductPath.c_str(), AZ::DataStream::ST_JSON, eventAsset.Get()) };

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

        AssetBuilderSDK::JobProduct eventAssetProduct{};
        eventAssetProduct.m_productFileName = absProductPath.String();
        eventAssetProduct.m_productAssetType = SaEventAsset::TYPEINFO_Uuid();
        eventAssetProduct.m_productSubID = SaEventAsset::AssetSubId;

        AssetBuilderSDK::OutputObject<SaEventAsset>(
            eventAsset.Get(),
            absProductPath.String().c_str(),
            AZ::AzTypeInfo<SaEventAsset>::Uuid(),
            SaEventAsset::AssetSubId,
            eventAssetProduct);

        response.m_outputProducts.emplace_back(AZStd::move(eventAssetProduct));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }

    void AudioEventAssetBuilderWorker::BuildFromAsset(
        AssetBuilderSDK::ProcessJobRequest const& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

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

        if (!CanBuildFromAsset(request, response))
        {
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

        AZ::IO::Path const sourceAssetPath{ request.m_fullPath };
        AZ_Info(AssetBuilderSDK::InfoWindow, "Full path: %s", sourceAssetPath.c_str());

        AZ::IO::Path const tempPath{ request.m_tempDirPath };
        AZ_Info(AssetBuilderSDK::InfoWindow, "Temp directory: %s", tempPath.c_str());

        AZ::IO::Path const eventProductPath = [&sourceAssetPath,
                                               &request]() -> decltype(eventProductPath)
        {
            AZ::IO::Path path{ request.m_tempDirPath };
            path /= sourceAssetPath.Filename();
            path.ReplaceExtension(SaEventAsset::Extension);
            return path;
        }();

        AZ_Info(AssetBuilderSDK::InfoWindow, "Product path: %s", eventProductPath.c_str());

        auto const eventProductAsset = AZ::Data::Asset<SaEventAsset>{
            AZ::Uuid::CreateRandom(),
            AZ::Utils::LoadObjectFromFile<SaEventAsset>(sourceAssetPath.c_str()),
            AZ::Data::AssetLoadBehavior::PreLoad
        };

        if (!eventProductAsset)
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow,
                false,
                "Failed to load source asset '%s'",
                sourceAssetPath.c_str());
            return;
        }

        if (AZ::Utils::SaveObjectToFile(
                eventProductPath.c_str(), AZ::DataStream::ST_XML, eventProductAsset.Get()))
        {
            AssetBuilderSDK::JobProduct eventProduct{};
            AssetBuilderSDK::OutputObject<SaEventAsset>(
                eventProductAsset.Get(),
                eventProductPath.String().c_str(),
                AZ::AzTypeInfo<SaEventAsset>::Uuid(),
                SaEventAsset::AssetSubId,
                eventProduct);

            response.m_outputProducts.push_back(eventProduct);
        }
        else
        {
            AZ_Error(
                AssetBuilderSDK::ErrorWindow, false, "Failed to save %s", eventProductPath.c_str());
            return;
        }

        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
}  // namespace SteamAudio
