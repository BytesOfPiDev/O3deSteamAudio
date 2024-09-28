#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Name/Name.h"

#include <SteamAudio/SteamAudioBus.h>

#include "AzFramework/Asset/GenericAssetHandler.h"
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

        static void Reflect(AZ::ReflectContext* context);

        SaSoundAsset();
        ~SaSoundAsset() override;

        static constexpr auto AssetGroup = "Sound";
        static constexpr auto AssetSubId = 1u;
        static constexpr auto ProductExtension{ "sasound" };

        AZStd::vector<AZ::u8> m_audioData;
    };

    using SaSoundAssetGenericHandler = AzFramework::GenericAssetHandler<SaSoundAsset>;

}  // namespace SteamAudio
