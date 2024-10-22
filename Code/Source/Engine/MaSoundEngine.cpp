#include "Engine/MaSoundEngine.h"

#include "AudioFileUtils.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Interface/Interface.h"
#include "AzCore/Module/Environment.h"
#include "AzCore/Outcome/Outcome.h"
#include "AzCore/PlatformDef.h"

#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/Id.h"
#include "Engine/SaAudioObjectBus.h"
#include "Engine/SaEventAsset.h"
#include "Engine/SaRegisteredEvent.h"

#include "Engine/Configuration.h"
#include "IAudioInterfacesCommonData.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace SteamAudio
{
    static constexpr auto HelloWorldAssetId{ "{959961C5-372D-4434-B836-202136CE8006}" };
    static constexpr auto MuteAllAssetId{ "{6CA6189A-7A36-4FCA-A801-BDF5911085BC}" };
    static constexpr auto UnmuteAllAssetId{ "{57D99443-73BD-49B3-AE9D-7135A64D92D9}" };
    static constexpr auto GetFocusAssetId{ "{70DE9CF7-8A00-40FD-B4DD-063B863B2626}" };
    static constexpr auto LoseFocusAssetId{ "{5542942A-848F-47A1-8B8D-3376914A6855}" };
    static constexpr auto DoNothingAssetId{ "{68C7A00D-4A9F-4AC5-A5B8-C481B9A60E38}" };

    static AZ::EnvironmentVariable<ma_engine*> s_maEngine{};  // NOLINT

    MaSoundEngine::MaSoundEngine()
    {
        AZ::Interface<ISoundEngine>::Register(this);
    }

    MaSoundEngine::~MaSoundEngine()
    {
        MaSoundEngine::Shutdown();
        AZ::Interface<ISoundEngine>::Unregister(this);
    }

    auto MaSoundEngine::Initialize() -> EngineNullOutcome
    {
        if (IsInitialized())
        {
            return AZ::Success();
        }

        m_contextSettings.version = STEAMAUDIO_VERSION;

        if (auto const outcome{ InitMiniAudio() }; !outcome.IsSuccess())
        {
            return AZ::Failure(AZStd::string::format(
                "Failed to initialize miniaudio: %s", outcome.GetError().c_str()));
        }

        m_soundLoader.Load();
        LoadNativeEvents();
        LoadEventAssets();

        // HACK: Tests might have created one manually. Fix later.
        if (!SaAudioObjectRequestBus::HasHandlers(GLOBAL_AUDIO_OBJECT_ID))
        {
            m_globalAudioObject.emplace(GLOBAL_AUDIO_OBJECT_ID);
        }

        m_initialized = true;
        AZ_Info(TYPEINFO_Name(), "Steam Audio Engine initialized.");
        return AZ::Success();
    }

    void MaSoundEngine::LoadNativeEvents()
    {
        if (!AZ::Data::AssetManager::IsReady())
        {
            AZ_Error(
                TYPEINFO_Name(),
                false,
                "The asset manager isn't ready. Unable to create native events");

            return;
        }

        AZ_Error(
            TYPEINFO_Name(),
            AZ::Data::AssetCatalogRequestBus::GetTotalNumOfEventHandlers() != 0,
            "No asset catalog available! Unable to register memory assets.");

        if (AZ::Data::AssetCatalogRequestBus::GetTotalNumOfEventHandlers() == 0)
        {
            return;
        }

        [this]() -> void
        {
            auto doNothingEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ DoNothingAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            doNothingEventAsset->SetEventName(Events::DoNothingEventName);

            m_registeredEvents.push_back(
                AZStd::make_unique<SaRegisteredEvent>(doNothingEventAsset));
        }();

        [this]() -> void
        {
            auto const helloWorldEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ HelloWorldAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            helloWorldEventAsset->SetEventName(Events::HelloWorldEventName);

            m_registeredEvents.push_back(
                AZStd::make_unique<SaRegisteredEvent>(helloWorldEventAsset));
        }();

        [this]()
        {
            auto const muteAllEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ MuteAllAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            muteAllEventAsset->SetEventName(Events::MuteAllEventName);

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(muteAllEventAsset));
        }();

        [this]() -> void
        {
            auto const unmuteAllEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ UnmuteAllAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            unmuteAllEventAsset->SetEventName(Events::UnmuteAllEventName);

            m_registeredEvents.push_back(
                AZStd::make_unique<SaRegisteredEvent>(unmuteAllEventAsset));
        }();

        [this]() -> void
        {
            auto const getFocusEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ GetFocusAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            getFocusEventAsset->SetEventName(Events::GetFocusEventName);

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(getFocusEventAsset));
        }();

        [this]() -> void
        {
            auto const loseFocusEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ LoseFocusAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            loseFocusEventAsset->SetEventName(Events::LoseFocusEventName);

            m_registeredEvents.push_back(
                AZStd::make_unique<SaRegisteredEvent>(loseFocusEventAsset));
        }();
    }

    void MaSoundEngine::LoadEventAssets()
    {
        auto const* const fileIo{ AZ::IO::FileIOBase::GetInstance() };

        auto const resolvedPathOutcome{ fileIo->ResolvePath(EventsAlias) };
        AZ_Error(
            AZ_FUNCTION_SIGNATURE,
            resolvedPathOutcome.has_value(),
            "Failed to resolve alias '%s'",
            EventsAlias);

        if (resolvedPathOutcome.has_value())
        {
            auto const files{ Audio::FindFilesInPath(
                resolvedPathOutcome.value().Native(), SaEventAsset::ExtensionWildcard) };

            AZ_Info(
                AZ_FUNCTION_SIGNATURE,
                "Found %lu events at '%s'",
                files.size(),
                resolvedPathOutcome.value().c_str());

            for (auto const& path : files)
            {
                AZ::Data::AssetType const assetType{ SaEventAsset::TYPEINFO_Uuid() };
                AZ::Data::AssetId assetIdResult{};
                AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                    assetIdResult,
                    &AZ::Data::AssetCatalogRequests::GetAssetIdByPath,
                    path.c_str(),
                    assetType,
                    true);

                if (!assetIdResult.IsValid())
                {
                    AZ_Warning(
                        AZ_FUNCTION_SIGNATURE,
                        false,
                        "Skipping '%s': unable to get asset id from its path.",
                        path.c_str());

                    continue;
                }

                auto& assetManager{ AZ::Data::AssetManager::Instance() };
                auto asset{ assetManager.GetAsset<SaEventAsset>(
                    assetIdResult,
                    AZ::Data::AssetLoadBehavior::QueueLoad,
                    AZ::Data::AssetLoadParameters{}) };

                if (asset.Get() == nullptr)
                {
                    AZ_Error(TYPEINFO_Name(), false, "Event Asset '%s' is nullptr");
                    continue;
                }
                else if (!asset.GetId().IsValid())
                {
                    AZ_Error(
                        TYPEINFO_Name(),
                        false,
                        "Event Asset '%s' loaded with invalid ID",
                        path.c_str());
                    continue;
                }

                asset.QueueLoad();
                asset.BlockUntilLoadComplete();

                if (!asset.IsReady())
                {
                    AZ_Warning(
                        TYPEINFO_Name(), false, "Event Asset '%s' failed to load", path.c_str());
                    continue;
                }

                m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(asset));
                AZLOG_INFO(
                    "Sound engine registered event '%s' w/ id '%llu'",
                    asset->GetEventName().c_str(),
                    static_cast<AZ::u64>(asset->GetEventId().GetValue()));

                asset = {};
            }
        }
    }

    auto MaSoundEngine::PostEvent(StartEventData const& startEventData) -> SaEventInstanceId
    {
        if (!startEventData.IsValid())
        {
            AZLOG_ERROR("Report event failed - start event data is not valid");
            return InvalidEventInstanceId;
        }

        SaEventInstanceId const eventInstanceId = [&startEventData]() -> decltype(eventInstanceId)
        {
            AZ_Error(
                "MaSoundEngine",
                SaAudioObjectRequestBus::HasHandlers(startEventData.m_gameObjectId),
                "PostEvent: no audio object request handler found w/ id '%llu'",
                startEventData.m_gameObjectId);

            auto result{ decltype(eventInstanceId){} };
            SaAudioObjectRequestBus::EventResult(
                result,
                startEventData.m_gameObjectId,
                &SaAudioObjectRequests::PushEvent,
                startEventData.m_eventId);

            return result;
        }();

        if (eventInstanceId == InvalidEventInstanceId)
        {
            AZLOG_ERROR(
                "Failed to report event '%llu' - unable to push event to audio object '%llu'",
                startEventData.m_eventId.GetValue(),
                startEventData.m_gameObjectId);
            return SaEventInstanceId{ InvalidInstanceId };
        }

        AZLOG(
            LOG_MaSoundEngine,
            "Successfully reported event '%llu'",
            startEventData.m_eventId.GetValue());

        return eventInstanceId;
    }

    auto MaSoundEngine::Shutdown() -> EngineNullOutcome
    {
        if (!IsInitialized())
        {
            return AZ::Success();
        }

        m_initialized = false;

        m_registeredObjects.clear();
        m_registeredEvents.clear();

        ShutdownMiniAudio();

        return AZ::Success();
    }

    auto MaSoundEngine::InitMiniAudio() -> EngineNullOutcome
    {
        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.channels = DefaultAudioChannels;
        engineConfig.sampleRate = DefaultSampleRate;
        engineConfig.listenerCount = 1;

        if (s_maEngine.IsConstructed())
        {
            return AZ::Failure("A miniaudio engine already exists! It should be nullptr.");
        }

        m_maEngine = AZStd::make_any<ma_engine>();

        s_maEngine = AZ::Environment::CreateVariable<ma_engine*>(
            s_lowLevelEngineEnvName, &AZStd::any_cast<ma_engine&>(m_maEngine));

        auto const initResult{ ma_engine_init(&engineConfig, s_maEngine.Get()) };
        if (initResult != MA_SUCCESS)
        {
            AZ_Error(
                "MaSoundEngine", false, "Failed to initialize ma_engine! Error: %zu", initResult);
            return AZ::Failure("Failed to init ma_engine");
        }

        return AZ::Success();
    }

    auto MaSoundEngine::ShutdownMiniAudio() -> EngineNullOutcome
    {
        AZLOG_INFO("Shutting down MiniAudio");

        if (!s_maEngine.IsConstructed())
        {
            return AZ::Failure("Environment variable for ma_engine is null");
        }

        ma_engine_uninit(s_maEngine.Get());

        s_maEngine.Reset();

        return AZ::Success();
    }

    void MaSoundEngine::Update(float /*deltaTime*/)
    {
    }

    auto MaSoundEngine::RegisterAudioObject(SaAudioObjectId const& objectId) -> EngineNullOutcome
    {
        if (SaAudioObjectRequestBus::HasHandlers(objectId))
        {
            return AZ::Failure("Object id is already in use");
        }

        m_registeredObjects.insert({ objectId, aznew SaAudioObject{ objectId } });

        if (!SaAudioObjectRequestBus::HasHandlers(objectId))
        {
            return AZ::Failure("Expected connection to bus did not occur");
        }

        return AZ::Success();
    }
}  // namespace SteamAudio
