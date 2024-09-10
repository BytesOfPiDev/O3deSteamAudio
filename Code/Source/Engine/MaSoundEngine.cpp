#include "Engine/MaSoundEngine.h"

#include "AudioFileUtils.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Interface/Interface.h"
#include "AzCore/Module/Environment.h"
#include "AzCore/PlatformDef.h"

#include "Engine/Common_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/Id.h"
#include "Engine/SaAudioObjectBus.h"
#include "Engine/SaEventAsset.h"
#include "Engine/SaRegisteredEvent.h"
#include "IAudioSystem.h"
#include "phonon.h"
#include "phonon_version.h"

#include "AzCore/Outcome/Outcome.h"
#include "AzFramework/Entity/GameEntityContextBus.h"

#include "Engine/Configuration.h"

#define MINIAUDIO_IMPLEMENTATION
#include "SteamAudio/MiniAudio.h"

[[maybe_unused]] static auto ma_result_from_IPLerror(IPLerror error) -> ma_result
{
    switch (error)
    {
    case IPL_STATUS_SUCCESS:
        return MA_SUCCESS;
    case IPL_STATUS_OUTOFMEMORY:
        return MA_OUT_OF_MEMORY;
    case IPL_STATUS_INITIALIZATION:
    case IPL_STATUS_FAILURE:
    default:
        return MA_ERROR;
    }
}

using ma_steamaudio_binaural_node_config = struct
{
    ma_node_config nodeConfig;
    ma_uint32 channelsIn;
    IPLAudioSettings iplAudioSettings;
    IPLContext iplContext;
    IPLHRTF iplHRTF; /* There is one HRTF object to many binaural effect objects. */
};

MA_API auto ma_steamaudio_binaural_node_config_init(
    ma_uint32 channelsIn,
    IPLAudioSettings iplAudioSettings,
    IPLContext iplContext,
    IPLHRTF iplHRTF) -> ma_steamaudio_binaural_node_config;

using ma_steamaudio_binaural_node = struct
{
    ma_node_base baseNode;
    IPLAudioSettings iplAudioSettings;
    IPLContext iplContext;
    IPLHRTF iplHRTF;
    IPLBinauralEffect iplEffect;
    ma_vec3f direction;
    float* ppBuffersIn[2]; /* Each buffer is an offset of _pHeap. */
    float* ppBuffersOut[2]; /* Each buffer is an offset of _pHeap. */
    void* _pHeap;
};

MA_API auto ma_steamaudio_binaural_node_init(
    ma_node_graph* pNodeGraph,
    ma_steamaudio_binaural_node_config const* pConfig,
    ma_allocation_callbacks const* pAllocationCallbacks,
    ma_steamaudio_binaural_node* pBinauralNode) -> ma_result;
MA_API void ma_steamaudio_binaural_node_uninit(
    ma_steamaudio_binaural_node* pBinauralNode,
    ma_allocation_callbacks const* pAllocationCallbacks);
MA_API auto ma_steamaudio_binaural_node_set_direction(
    ma_steamaudio_binaural_node* pBinauralNode, float x, float y, float z) -> ma_result;

MA_API auto ma_steamaudio_binaural_node_config_init(
    ma_uint32 channelsIn,
    IPLAudioSettings iplAudioSettings,
    IPLContext iplContext,
    IPLHRTF iplHRTF) -> ma_steamaudio_binaural_node_config
{
    ma_steamaudio_binaural_node_config config;

    MA_ZERO_OBJECT(&config);
    config.nodeConfig = ma_node_config_init();
    config.channelsIn = channelsIn;
    config.iplAudioSettings = iplAudioSettings;
    config.iplContext = iplContext;
    config.iplHRTF = iplHRTF;

    return config;
}

static void ma_steamaudio_binaural_node_process_pcm_frames(
    ma_node* pNode,
    float const** ppFramesIn,
    ma_uint32* pFrameCountIn,
    float** ppFramesOut,
    ma_uint32* pFrameCountOut)
{
    auto* pBinauralNode = (ma_steamaudio_binaural_node*)pNode;
    IPLBinauralEffectParams binauralParams;
    IPLAudioBuffer inputBufferDesc;
    IPLAudioBuffer outputBufferDesc;
    ma_uint32 totalFramesToProcess = *pFrameCountOut;
    ma_uint32 totalFramesProcessed = 0;

    binauralParams.direction.x = pBinauralNode->direction.x;
    binauralParams.direction.y = pBinauralNode->direction.y;
    binauralParams.direction.z = pBinauralNode->direction.z;
    binauralParams.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
    binauralParams.spatialBlend = 1.0f;
    binauralParams.hrtf = pBinauralNode->iplHRTF;

    inputBufferDesc.numChannels = (IPLint32)ma_node_get_input_channels(pNode, 0);

    /* We'll run this in a loop just in case our deinterleaved buffers are too small. */
    outputBufferDesc.numSamples = pBinauralNode->iplAudioSettings.frameSize;
    outputBufferDesc.numChannels = 2;
    outputBufferDesc.data = pBinauralNode->ppBuffersOut;

    while (totalFramesProcessed < totalFramesToProcess)
    {
        ma_uint32 framesToProcessThisIteration = totalFramesToProcess - totalFramesProcessed;
        if (framesToProcessThisIteration > (ma_uint32)pBinauralNode->iplAudioSettings.frameSize)
        {
            framesToProcessThisIteration = (ma_uint32)pBinauralNode->iplAudioSettings.frameSize;
        }

        if (inputBufferDesc.numChannels == 1)
        {
            /* Fast path. No need for deinterleaving since it's a mono stream. */
            pBinauralNode->ppBuffersIn[0] =
                (float*)ma_offset_pcm_frames_const_ptr_f32(ppFramesIn[0], totalFramesProcessed, 1);
        }
        else
        {
            /* Slow path. Need to deinterleave the input data. */
            ma_deinterleave_pcm_frames(
                ma_format_f32,
                inputBufferDesc.numChannels,
                framesToProcessThisIteration,
                ma_offset_pcm_frames_const_ptr_f32(
                    ppFramesIn[0], totalFramesProcessed, inputBufferDesc.numChannels),
                reinterpret_cast<void**>(pBinauralNode->ppBuffersIn));
        }

        inputBufferDesc.data = pBinauralNode->ppBuffersIn;
        inputBufferDesc.numSamples = (IPLint32)framesToProcessThisIteration;

        /* Apply the effect. */
        iplBinauralEffectApply(
            pBinauralNode->iplEffect, &binauralParams, &inputBufferDesc, &outputBufferDesc);

        /* Interleave straight into the output buffer. */
        ma_interleave_pcm_frames(
            ma_format_f32,
            2,
            framesToProcessThisIteration,
            (void const**)(pBinauralNode->ppBuffersOut),
            reinterpret_cast<void*>(
                ma_offset_pcm_frames_ptr_f32(ppFramesOut[0], totalFramesProcessed, 2)));

        /* Advance. */
        totalFramesProcessed += framesToProcessThisIteration;
    }

    (void)pFrameCountIn; /* Unused. */
}

static ma_node_vtable g_ma_steamaudio_binaural_node_vtable = {
    ma_steamaudio_binaural_node_process_pcm_frames,
    NULL,
    1, /* 1 input channel. */
    1, /* 1 output channel. */
    0
};

MA_API auto ma_steamaudio_binaural_node_init(
    ma_node_graph* pNodeGraph,
    ma_steamaudio_binaural_node_config const* pConfig,
    ma_allocation_callbacks const* pAllocationCallbacks,
    ma_steamaudio_binaural_node* pBinauralNode) -> ma_result
{
    ma_result result;
    ma_node_config baseConfig;
    ma_uint32 channelsIn;
    ma_uint32 channelsOut;
    IPLBinauralEffectSettings iplBinauralEffectSettings;
    size_t heapSizeInBytes;

    if (pBinauralNode == NULL)
    {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pBinauralNode);

    if (pConfig == NULL || pConfig->iplAudioSettings.frameSize == 0 ||
        pConfig->iplContext == NULL || pConfig->iplHRTF == NULL)
    {
        return MA_INVALID_ARGS;
    }

    /* Steam Audio only supports mono and stereo input. */
    if (pConfig->channelsIn < 1 || pConfig->channelsIn > 2)
    {
        return MA_INVALID_ARGS;
    }

    channelsIn = pConfig->channelsIn;
    channelsOut = 2; /* Always stereo output. */

    baseConfig = ma_node_config_init();
    baseConfig.vtable = &g_ma_steamaudio_binaural_node_vtable;
    baseConfig.pInputChannels = &channelsIn;
    baseConfig.pOutputChannels = &channelsOut;
    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pBinauralNode->baseNode);
    if (result != MA_SUCCESS)
    {
        return result;
    }

    pBinauralNode->iplAudioSettings = pConfig->iplAudioSettings;
    pBinauralNode->iplContext = pConfig->iplContext;
    pBinauralNode->iplHRTF = pConfig->iplHRTF;

    MA_ZERO_OBJECT(&iplBinauralEffectSettings);
    iplBinauralEffectSettings.hrtf = pBinauralNode->iplHRTF;

    result = ma_result_from_IPLerror(iplBinauralEffectCreate(
        pBinauralNode->iplContext,
        &pBinauralNode->iplAudioSettings,
        &iplBinauralEffectSettings,
        &pBinauralNode->iplEffect));
    if (result != MA_SUCCESS)
    {
        ma_node_uninit(&pBinauralNode->baseNode, pAllocationCallbacks);
        return result;
    }

    heapSizeInBytes = 0;

    /*
    Unfortunately Steam Audio uses deinterleaved buffers for everything so we'll need to use some
    intermediary buffers. We'll allocate one big buffer on the heap and then use offsets. We'll
    use the frame size from the IPLAudioSettings structure as a basis for the size of the buffer.
    */
    heapSizeInBytes += sizeof(float) * channelsOut *
        pBinauralNode->iplAudioSettings.frameSize; /* Output buffer. */
    heapSizeInBytes +=
        sizeof(float) * channelsIn * pBinauralNode->iplAudioSettings.frameSize; /* Input buffer. */

    pBinauralNode->_pHeap = ma_malloc(heapSizeInBytes, pAllocationCallbacks);
    if (pBinauralNode->_pHeap == nullptr)
    {
        iplBinauralEffectRelease(&pBinauralNode->iplEffect);
        ma_node_uninit(&pBinauralNode->baseNode, pAllocationCallbacks);
        return MA_OUT_OF_MEMORY;
    }

    pBinauralNode->ppBuffersOut[0] = (float*)pBinauralNode->_pHeap;
    pBinauralNode->ppBuffersOut[1] = (float*)ma_offset_ptr(
        pBinauralNode->_pHeap, sizeof(float) * pBinauralNode->iplAudioSettings.frameSize);

    {
        ma_uint32 iChannelIn;
        for (iChannelIn = 0; iChannelIn < channelsIn; iChannelIn += 1)
        {
            pBinauralNode->ppBuffersIn[iChannelIn] = (float*)ma_offset_ptr(
                pBinauralNode->_pHeap,
                sizeof(float) * pBinauralNode->iplAudioSettings.frameSize *
                    (channelsOut + iChannelIn));
        }
    }

    return MA_SUCCESS;
}

MA_API void ma_steamaudio_binaural_node_uninit(
    ma_steamaudio_binaural_node* pBinauralNode, ma_allocation_callbacks const* pAllocationCallbacks)
{
    if (pBinauralNode == nullptr)
    {
        return;
    }

    /* The base node is always uninitialized first. */
    ma_node_uninit(&pBinauralNode->baseNode, pAllocationCallbacks);

    /*
    The Steam Audio objects are deleted after the base node. This ensures the base node is removed
    from the graph first to ensure these objects aren't getting used by the audio thread.
    */
    iplBinauralEffectRelease(&pBinauralNode->iplEffect);
    ma_free(pBinauralNode->_pHeap, pAllocationCallbacks);
}

MA_API auto ma_steamaudio_binaural_node_set_direction(
    ma_steamaudio_binaural_node* pBinauralNode, float x, float y, float z) -> ma_result
{
    if (pBinauralNode == NULL)
    {
        return MA_INVALID_ARGS;
    }

    pBinauralNode->direction.x = x;
    pBinauralNode->direction.y = y;
    pBinauralNode->direction.z = z;

    return MA_SUCCESS;
}

namespace SteamAudio
{
    static constexpr auto HelloWorldAssetId{ "959961C5-372D-4434-B836-202136CE8006" };
    static constexpr auto MuteAllAssetId{ "6CA6189A-7A36-4FCA-A801-BDF5911085BC" };
    static constexpr auto UnmuteAllAssetId{ "57D99443-73BD-49B3-AE9D-7135A64D92D9" };
    static constexpr auto GetFocusAssetId{ "70DE9CF7-8A00-40FD-B4DD-063B863B2626" };
    static constexpr auto LoseFocusAssetId{ "5542942A-848F-47A1-8B8D-3376914A6855" };
    static constexpr auto DoNothingAssetId{ "68C7A00D-4A9F-4AC5-A5B8-C481B9A60E38" };

    static AZ::EnvironmentVariable<ma_engine*> s_maEngine{};  // NOLINT

    MaSoundEngine::MaSoundEngine()
    {
        AZ::Interface<ISoundEngine>::Register(this);
    }

    MaSoundEngine::~MaSoundEngine()
    {
        Shutdown();
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

        LoadNativeEvents();
        LoadEventAssets();

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

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::DoNothingEventName),
                doNothingEventAsset));
        }();

        [this]() -> void
        {
            auto const helloWorldEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ HelloWorldAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::HelloWorldEventName),
                helloWorldEventAsset));
        }();

        [this]()
        {
            auto const muteAllEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ MuteAllAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::MuteAllEventName), muteAllEventAsset));
        }();

        [this]() -> void
        {
            auto const unmuteAllEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ UnmuteAllAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::UnmuteAllEventName),
                unmuteAllEventAsset));
        }();

        [this]() -> void
        {
            auto const getFocusEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ GetFocusAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::GetFocusEventName), getFocusEventAsset));
        }();

        [this]() -> void
        {
            auto const loseFocusEventAsset{
                AZ::Data::AssetManager::Instance().FindOrCreateAsset<SaEventAsset>(
                    AZ::Data::AssetId{ LoseFocusAssetId }, AZ::Data::AssetLoadBehavior::NoLoad)
            };

            m_registeredEvents.push_back(AZStd::make_unique<SaRegisteredEvent>(
                Audio::AudioStringToID<SaEventId>(Events::LoseFocusEventName),
                loseFocusEventAsset));
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

                AZLOG_ERROR(
                    "Adding audio event [Name: %s | Id: %llu",
                    asset->GetEventName().c_str(),
                    static_cast<AZ::u64>(asset->GetEventId()));

                m_registeredEvents.push_back(
                    AZStd::make_unique<SaRegisteredEvent>(asset->GetEventId(), asset));

                asset = {};
            }
        }
    }

    auto MaSoundEngine::ReportEvent(StartEventData const& startEventData) -> SaEventInstanceId
    {
        if (startEventData.m_eventId == InvalidEventId)
        {
            AZLOG_ERROR("Report event failed because the event id is invalid.");
            return InvalidEventInstanceId;
        }

        SaEventInstanceId const eventInstanceId = [&startEventData]() -> decltype(eventInstanceId)
        {
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
                "ReportEvent failed because we could not push the event to an audio object");
            return SaEventInstanceId{ InvalidInstanceId };
        }

        AZLOG(
            LOG_MaSoundEngine,
            "Report event succeeded. Name: %s | Id: %llu",
            startEventData.m_eventName.GetCStr(),
            static_cast<AZ::u64>(startEventData.m_eventId));

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

    auto MaSoundEngine::RegisterAudioObject(SaGameObjectId const& objectId) -> EngineNullOutcome
    {
        AZ::Entity* entity{};

        AZ::ComponentApplicationBus::BroadcastResult(
            entity,
            &AZ::ComponentApplicationBus::Events::FindEntity,
            static_cast<AZ::EntityId>(objectId));

        m_registeredObjects.insert({ objectId, aznew AudioObject{} });

        AZ_Error(AZ_FUNCTION_SIGNATURE, false, "Not implemented.");
        return AZ::Failure("Not implemented.");
    }

    void MaSoundEngine::LoadSounds()
    {
        m_soundLoader.Load();
    }
}  // namespace SteamAudio
