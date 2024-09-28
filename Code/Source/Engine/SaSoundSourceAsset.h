#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "Engine/SaSoundAsset.h"
#include "Engine/SoundConfig.h"

namespace SteamAudio
{
    class SaSoundSourceAssetHandler;

    class SaSoundSourceData : public AZ::Data::AssetData
    {
        friend SaSoundSourceAssetHandler;

    public:
        AZ_CLASS_ALLOCATOR(SaSoundSourceData, AZ::SystemAllocator);
        AZ_DISABLE_COPY_MOVE(SaSoundSourceData);
        AZ_RTTI(SaSoundSourceData, "{CD038A39-CF2C-4A69-9877-DC7EFE284F25}", AZ::Data::AssetData);

        static constexpr auto CurrentVersion{ 3 };
        static constexpr auto Extension{ "sasoundsource" };
        static constexpr auto ExtensionWildcard{ "*.sasoundsource" };

        static void Reflect(AZ::ReflectContext* reflect);

        SaSoundSourceData() = default;
        ~SaSoundSourceData() override = default;

        [[nodiscard]] auto GetSoundSourceName() const -> AZ::Name
        {
            return m_name;
        }

        [[nodiscard]] auto GetSoundAsset() const -> AZ::Data::Asset<SaSoundAsset> const&
        {
            return m_asset;
        }

    protected:
        void OnSoundAssetChanged();
        void OnNameChanged();

        void LoadSoundSourceDeps()
        {
            m_asset.QueueLoad();
            m_asset.BlockUntilLoadComplete();
        }

    private:
        AZ::Data::Asset<SaSoundAsset> m_asset{ AZ::Data::AssetLoadBehavior::PreLoad };
        SoundConfig m_soundSettings{};

        AZ::Name m_name{};
        AZStd::string m_nameStr;  // FIXME: Use a variant or figure out how to get the edit context
                                  // to use AZ::Name as strings
    };

    using SaSoundSourceAssetPtr = AZ::Data::Asset<SaSoundSourceData>;

}  // namespace SteamAudio
