#pragma once

#include "AudioAllocators.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Name/Name.h"
#include "IAudioInterfacesCommonData.h"
namespace SteamAudio
{
    class ISaSoundAsset
    {
    public:
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

    class SaSoundAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(SaSoundAsset);
        AZ_DISABLE_COPY_MOVE(SaSoundAsset);
        AZ_CLASS_ALLOCATOR_DECL;

        using Buffer = AZStd::vector<AZStd::byte, Audio::AudioImplStdAllocator>;

        static void Reflect(AZ::ReflectContext* context);

        SaSoundAsset();
        SaSoundAsset(
            Audio::AudioInputSourceType sourceType,
            Audio::AudioInputSampleType sampleType,
            Buffer buffer);
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

        [[nodiscard]] auto GetBuffer() const -> AZStd::span<AZStd::byte const>
        {
            return m_buffer;
        }

        [[nodiscard]] auto GetSourceType() const
        {
            return m_sourceType;
        }
        [[nodiscard]] auto GetSampleType() const
        {
            return m_sampleType;
        }

    private:
        Audio::AudioInputSourceType m_sourceType{ Audio::AudioInputSourceType::Unsupported };
        Audio::AudioInputSampleType m_sampleType{ Audio::AudioInputSampleType::Unsupported };
        Buffer m_buffer{};
    };

}  // namespace SteamAudio
