#include "Engine/SoundEngine.h"

#include "phonon.h"
#include "phonon_version.h"

#include "AudioAllocators.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Outcome/Outcome.h"
#include "AzFramework/Entity/GameEntityContextBus.h"

#include "Engine/Configuration.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#undef MINIAUDIO_IMPLEMENTATION

static auto my_malloc(size_t size, size_t alignment) -> void*
{
    return azmalloc(size, alignment, Audio::AudioImplAllocator);
}

static void my_free(void* block)
{
    azfree(block, Audio::AudioImplAllocator);
}

static auto GetDecoder() -> ma_decoder&
{
    static ma_decoder instance{};
    return instance;
}

static auto GetDevice() -> ma_device&
{
    static ma_device instance{};
    return instance;
}

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
    if (pBinauralNode->_pHeap == NULL)
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

    auto SteamAudioEngine::Initialize() -> EngineNullOutcome
    {
        m_contextSettings.version = STEAMAUDIO_VERSION;
        m_context = nullptr;

        IPLerror errorCode = iplContextCreate(&m_contextSettings, &m_context);
        if (errorCode)
        {
            return AZ::Failure("Unable to create Steam Audio context. Error code: %d");
        }

        m_contextSettings.allocateCallback = my_malloc;
        m_contextSettings.freeCallback = my_free;

        m_hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        m_hrtfSettings.volume = 1.0f;

        m_audioSettings.samplingRate = DefaultSampleRate;
        m_audioSettings.frameSize = DefaultFrameSize;

        iplHRTFCreate(m_context, &m_audioSettings, &m_hrtfSettings, &m_hrtf);

        m_sceneSettings.type = IPL_SCENETYPE_DEFAULT;
        iplSceneCreate(m_context, &m_sceneSettings, &m_scene);

        m_simulationSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
        m_simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;

        iplSimulatorCreate(m_context, &m_simulationSettings, &m_simulator);

        iplSimulatorSetScene(m_simulator, m_scene);
        iplSimulatorCommit(m_simulator);

        m_sharedInputs.listener = m_listenerCoordinates;
        iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_DIRECT, &m_sharedInputs);

        AZ_Info(TYPEINFO_Name(), "Steam Audio Engine initialized.");
        return AZ::Success();
    }

    auto SteamAudioEngine::Shutdown() -> EngineNullOutcome
    {
        if (m_context)
        {
            iplContextRelease(&m_context);
            m_context = nullptr;
        }

        ma_device_uninit(&GetDevice());
        ma_decoder_uninit(&GetDecoder());
        return AZ::Success();
    }

    void SteamAudioEngine::InitMiniAudio()
    {
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = 2;
        config.sampleRate = DefaultSampleRate;
        config.dataCallback = []([[maybe_unused]] ma_device* pDevice,
                                 [[maybe_unused]] void* pOutput,
                                 [[maybe_unused]] void const* pInput,
                                 [[maybe_unused]] ma_uint32 frameCount)
        {
            auto* pDecoder = (ma_decoder*)pDevice->pUserData;
            if (pDecoder == nullptr)
            {
                return;
            }

            ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, nullptr);
        };

        config.pUserData = &GetDecoder();

        if (ma_device_init(nullptr, &config, &GetDevice()) != MA_SUCCESS)
        {
            ma_decoder_uninit(&GetDecoder());
            AZ_Error(TYPEINFO_Name(), false, "Failed to initialize miniaudio device!");
            return;
        }

        if (ma_device_start(&GetDevice()) != MA_SUCCESS)
        {
            ma_device_uninit(&GetDevice());
            ma_decoder_uninit(&GetDecoder());
            AZ_Error(TYPEINFO_Name(), false, "Failed to start an initialized miniaudio device!");
        };

        AZLOG_INFO(TYPEINFO_Name(), " the miniaudio system.\n");
    }

    void SteamAudioEngine::Update(float /*deltaTime*/)
    {
        iplSimulatorRunDirect(m_simulator);
    }

    auto SteamAudioEngine::RegisterAudioObject(SaGameObjectId const& objectId) -> EngineNullOutcome
    {
        AZ::Entity* entity{};

        AZ::ComponentApplicationBus::BroadcastResult(
            entity, &AZ::ComponentApplicationBus::Events::FindEntity, objectId);

        auto gameObject{ AudioObject(objectId, m_simulator) };

        return AZ::Failure("Not implemented.");
    }

} // namespace SteamAudio
