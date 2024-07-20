#pragma once

#include "IAudioInterfacesCommonData.h"

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Name/Name.h"

#include <SteamAudio/SteamAudioBus.h>

#include "Engine/Configuration.h"
#include "Engine/ISoundEngine.h"

extern "C" {
struct ma_engine;
struct ma_sound;
}

namespace SteamAudio
{
    class ISaSoundAsset
    {
    public:
        AZ_DISABLE_COPY_MOVE(ISaSoundAsset);

        ISaSoundAsset() = default;
        virtual ~ISaSoundAsset() = default;
        [[nodiscard]] virtual auto CopyBuffer() const -> AZStd::vector<float>
        {
            return {};
        }
    };

    struct SaSoundAssetBusTraits : public AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Name;
        using BusHandlerOrderCompare = AZ::BusHandlerCompareDefault;
        using MutexType = AZStd::mutex;

        static constexpr bool EnableEventQueue = false;
        static constexpr bool EventQueueingActiveByDefault = true;
        static constexpr bool EnableQueuedReferences = false;

        using EventQueueMutexType = AZ::NullMutex;
    };

    using SaSoundAssetRequestBus = AZ::EBus<ISaSoundAsset, SaSoundAssetBusTraits>;

    class SaSoundAsset
        : public AZ::Data::AssetData
        , protected SoundEngineNotificationBus::Handler
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(SaSoundAsset);
        AZ_DISABLE_COPY_MOVE(SaSoundAsset);
        AZ_CLASS_ALLOCATOR_DECL;

        using ByteBuffer = AZStd::vector<AZ::u8>;
        using ByteBufferView = AZStd::span<AZ::u8 const>;
        using Buffer = ByteBuffer;
        using BufferView = ByteBufferView;

        struct ISoundImpl
        {
        };

        static void Reflect(AZ::ReflectContext* context);

        SaSoundAsset();
        SaSoundAsset(
            Audio::AudioInputSourceType sourceType,
            Audio::AudioInputSampleType sampleType,
            AZ::u32 channelCount,
            AZ::u32 sampleRate,
            Buffer buffer);

        SaSoundAsset(
            Audio::AudioInputSourceType sourceType,
            Audio::AudioInputSampleType sampleType,
            AZ::u32 channelCount,
            AZ::u32 sampleRate,
            BufferView buffer);

        ~SaSoundAsset() override;

        static constexpr auto ProductExtension{ "sasound" };
        static constexpr auto ProductExtensionWildcard{ "*.sasound" };

        static constexpr auto RawExtension{ "raw" };
        static constexpr auto RawExtensionWildcard{ "*.raw" };
        static constexpr auto RawExtensionRegex{ R"((.*sounds\/steamaudio\/*\/).*\.raw)" };

        static constexpr auto WavExtension{ "wav" };
        static constexpr auto WavExtensionWildcard{ "*.wav" };
        static constexpr auto WavExtensionRegex{ R"((.*sounds\/steamaudio\/*\/).*\.wav)" };

        static constexpr auto OggExtension{ "ogg" };
        static constexpr auto OggExtensionWildcard{ "*.ogg" };
        static constexpr auto OggExtensionRegex{ R"((.*sounds\/steamaudio\/*\/).*\.ogg)" };

        static constexpr auto AssetGroup = "Sound";
        static constexpr auto AssetSubId = 1u;

        [[nodiscard]] auto GetBuffer() const -> BufferView
        {
            return m_buffer;
        }

        /// Set encoded data
        void SetAudioData(Audio::AudioInputSourceType sourceType, Buffer buffer);

        /// Set encoded data
        void SetAudioData(Audio::AudioInputSourceType sourceType, BufferView buffer);

        /// Set decoded data
        void SetAudioData(
            Audio::AudioInputSourceType sourceType,
            Audio::AudioInputSampleType sampleType,
            AZ::u32 channelCount,
            AZ::u32 sampleRate,
            Buffer buffer);

        /// Set decoded data
        void SetAudioData(
            Audio::AudioInputSourceType sourceType,
            Audio::AudioInputSampleType sampleType,
            AZ::u32 channelCount,
            AZ::u32 sampleRate,
            BufferView buffer);

        [[nodiscard]] auto GetSourceType() const -> Audio::AudioInputSourceType
        {
            return m_sourceType;
        }

        [[nodiscard]] auto GetSampleType() const -> Audio::AudioInputSampleType
        {
            return m_sampleType;
        }

        [[nodiscard]] auto GetChannelCount() const -> AZ::u32
        {
            return m_channels;
        }

        [[nodiscard]] auto GetSampleRate() const -> AZ::u32
        {
            return m_sampleRate;
        }

        [[nodiscard]] auto GetFrameCount() const -> AZ::u64
        {
            return m_buffer.size() / sizeof(float);
        }

    private:
        Audio::AudioInputSourceType m_sourceType{ Audio::AudioInputSourceType::Unsupported };
        Audio::AudioInputSampleType m_sampleType{ Audio::AudioInputSampleType::Unsupported };
        Buffer m_buffer{};
        AZ::u32 m_channels{ DefaultAudioChannels };
        AZ::u32 m_sampleRate{ DefaultSampleRate };

        AZStd::unique_ptr<ISoundImpl> m_sound{};
    };

}  // namespace SteamAudio
