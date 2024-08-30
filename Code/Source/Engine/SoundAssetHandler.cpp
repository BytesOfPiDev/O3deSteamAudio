#include "Engine/SoundAssetHandler.h"

#include "AudioAllocators.h"
#include "AzCore/Component/ComponentApplicationBus.h"
#include "AzCore/Serialization/Utils.h"

#include "Engine/SaSoundAsset.h"
#include "SteamAudio/SteamAudioTypeIds.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(SaSoundAssetHandler, Audio::AudioImplAllocator);
    AZ_RTTI_NO_TYPE_INFO_IMPL(SaSoundAssetHandler, AZ::Data::AssetHandler);
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        SaSoundAssetHandler, "SaSoundAssetHandler", SaSoundAssetHandlerTypeId);

    SaSoundAssetHandler::SaSoundAssetHandler() = default;

    SaSoundAssetHandler::~SaSoundAssetHandler() = default;

    auto SteamAudio::SaSoundAssetHandler::CreateAsset(
        const AZ::Data::AssetId& /*id*/, const AZ::Data::AssetType& type) -> AZ::Data::AssetPtr
    {
        if (type == AZ::AzTypeInfo<SaSoundAsset>::Uuid())
        {
            return aznew SaSoundAsset();
        }

        AZ_Error("SaSoundAssetHandler", false, "This handler deals only with SaSoundAsset type.");
        return nullptr;
    }

    auto SaSoundAssetHandler::LoadAssetData(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset,
        AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
        AZ::Data::AssetFilterCB const& assetLoadFilterCB) -> AZ::Data::AssetHandler::LoadResult
    {
        bool const result = AZ::Utils::LoadObjectFromStreamInPlace<SaSoundAsset>(
            *stream, *asset.GetAs<SaSoundAsset>());
        if (result == false)
        {
            AZ_Error(__FUNCTION__, false, "Failed to load asset");
            return AssetHandler::LoadResult::Error;
        }

        return AssetHandler::LoadResult::LoadComplete;
    }

    void SaSoundAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
    {
        delete ptr;
    }

    void SaSoundAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes)
    {
        assetTypes.push_back(AZ::AzTypeInfo<SaSoundAsset>::Uuid());
    }

    auto SaSoundAssetHandler::GetAssetType() const -> AZ::Data::AssetType
    {
        return AZ::Data::AssetType{ SaSoundAssetTypeId };
    }

    void SaSoundAssetHandler::GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions)
    {
        extensions.push_back(SaSoundAsset::ProductExtension);
    }

    auto SaSoundAssetHandler::GetAssetTypeDisplayName() const -> char const*
    {
        return "Sound Asset (SteamAudio Gem)";
    }
    auto SaSoundAssetHandler::GetBrowserIcon() const -> char const*
    {
        return "Icons/Components/ColliderMesh.svg";
    }

    auto SaSoundAssetHandler::GetGroup() const -> char const*
    {
        return "Sound";
    }

    auto SaSoundAssetHandler::GetComponentTypeId() const -> AZ::Uuid
    {
        return {};
    }

    auto SaSoundAssetHandler::CanCreateComponent(AZ::Data::AssetId const& /*assetId*/) const -> bool
    {
        return false;
    }
    void SaSoundAssetHandler::Register()
    {
        AZ::ComponentApplicationBus::BroadcastResult(
            m_serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        AZ::Data::AssetCatalogRequestBus::Broadcast(
            &AZ::Data::AssetCatalogRequestBus::Events::EnableCatalogForAsset,
            AZ::Data::AssetType{ SaSoundAssetTypeId });
        AZ::Data::AssetCatalogRequestBus::Broadcast(
            &AZ::Data::AssetCatalogRequestBus::Events::AddExtension,
            SaSoundAsset::ProductExtension);

        AZ_Assert(AZ::Data::AssetManager::IsReady(), "AssetManager isn't ready!");
        AZ::Data::AssetManager::Instance().RegisterHandler(
            this, AZ::Data::AssetType{ SaSoundAssetTypeId });
    }

    void SaSoundAssetHandler::Unregister()
    {
        if (AZ::Data::AssetManager::IsReady())
        {
            AZ::Data::AssetManager::Instance().UnregisterHandler(this);
        }
    }

    void SaSoundAssetHandler::OnSoundManagerReady() const
    {
    }

    void SaSoundAssetHandler::OnEventmanagerReady() const
    {
    }
}  // namespace SteamAudio
