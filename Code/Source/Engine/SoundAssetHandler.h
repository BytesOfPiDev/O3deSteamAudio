#pragma once

#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetTypeInfoBus.h"
#include "Engine/ISoundEngine.h"

namespace SteamAudio
{
    class SaSoundAssetHandler
        : public AZ::Data::AssetHandler
        , public AZ::AssetTypeInfoBus::Handler
        , protected SoundEngineNotificationBus::Handler
    {
        using Base = AZ::Data::AssetHandler;

    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(SaSoundAssetHandler);
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(SaSoundAssetHandler);

        SaSoundAssetHandler();
        ~SaSoundAssetHandler() override;

        void Register();
        void Unregister();

        // AZ::Data::AssetHandler
        auto CreateAsset(const AZ::Data::AssetId& id, const AZ::Data::AssetType& type)
            -> AZ::Data::AssetPtr override;
        auto LoadAssetData(
            const AZ::Data::Asset<AZ::Data::AssetData>& asset,
            AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
            const AZ::Data::AssetFilterCB& assetLoadFilterCB)
            -> AZ::Data::AssetHandler::LoadResult override;
        void DestroyAsset(AZ::Data::AssetPtr ptr) override;
        void GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes) override;

        // AZ::AssetTypeInfoBus
        [[nodiscard]] AZ::Data::AssetType GetAssetType() const override;
        void GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions) override;
        [[nodiscard]] auto GetAssetTypeDisplayName() const -> char const* override;
        [[nodiscard]] auto GetBrowserIcon() const -> char const* override;
        [[nodiscard]] auto GetGroup() const -> char const* override;
        [[nodiscard]] auto GetComponentTypeId() const -> AZ::Uuid override;
        [[nodiscard]] auto CanCreateComponent(const AZ::Data::AssetId& assetId) const
            -> bool override;

    protected:
        void OnSoundManagerReady() const override;
        void OnEventmanagerReady() const override;

    private:
        AZ::SerializeContext* m_serializeContext{};
    };
}  // namespace SteamAudio
