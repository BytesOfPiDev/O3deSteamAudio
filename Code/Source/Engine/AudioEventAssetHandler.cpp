#include "Engine/AudioEventAssetHandler.h"

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetTypeInfoBus.h"
#include "AzCore/Component/ComponentApplicationBus.h"
#include "AzCore/IO/GenericStreams.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/Serialization/Utils.h"
#include "AzCore/std/smart_ptr/unique_ptr.h"
#include "AzFramework/Asset/GenericAssetHandler.h"

#include "Engine/AudioEvent.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/ISoundEngine.h"
#include "SteamAudio/SteamAudioTypeIds.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(SaEventAssetHandler, AZ::SystemAllocator);
    AZ_RTTI_NO_TYPE_INFO_IMPL(
        SaEventAssetHandler, AzFramework::GenericAssetHandlerBase, AZ::Data::AssetHandler);
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        SaEventAssetHandler, "SteamAudio Event Asset Handler", SaEventAssetHandlerTypeId);

    SaEventAssetHandler::SaEventAssetHandler()
    {
        AZ::AssetTypeInfoBus::Handler::BusConnect(AZ::Data::AssetType{ SaEventAssetTypeId });
    }

    SaEventAssetHandler::~SaEventAssetHandler()
    {
        AZ::AssetTypeInfoBus::Handler::BusDisconnect();
    }

    void SaEventAssetHandler::Register()
    {
        AZ::ComponentApplicationBus::BroadcastResult(
            m_serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        AZ::Data::AssetCatalogRequestBus::Broadcast(
            &AZ::Data::AssetCatalogRequestBus::Events::EnableCatalogForAsset,
            AZ::Data::AssetType{ SaEventAssetTypeId });

        AZ::Data::AssetCatalogRequestBus::Broadcast(
            &AZ::Data::AssetCatalogRequestBus::Events::AddExtension,
            SaEventAsset::ProductExtensionWildcard);

        AZ_Assert(AZ::Data::AssetManager::IsReady(), "AssetManager isn't ready!");
        AZ::Data::AssetManager::Instance().RegisterHandler(
            this, AZ::Data::AssetType{ SaEventAssetTypeId });
    }

    void SaEventAssetHandler::Unregister()
    {
        if (AZ::Data::AssetManager::IsReady())
        {
            AZ::Data::AssetManager::Instance().UnregisterHandler(this);
        }
    }

    auto SaEventAssetHandler::CanHandleAsset(AZ::Data::AssetId const& id) const -> bool
    {
        AZ::IO::Path const assetPath = Util::GetAssetPath(id);
        return assetPath.Match(SaEventAsset::ProductExtensionWildcard);
    }

    void SaEventAssetHandler::InitAsset(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset,
        bool /*loadStageSucceeded*/,
        bool /*isReload*/)
    {
        AZ::Data::AssetManagerBus::Broadcast(
            &AZ::Data::AssetManagerBus::Events::OnAssetReady, asset);
    }

    auto SaEventAssetHandler::CreateAsset(
        AZ::Data::AssetId const& /*id*/, AZ::Data::AssetType const& type) -> AZ::Data::AssetPtr
    {
        if (type == AZ::Data::AssetType{ SaEventAssetTypeId })
        {
            return aznew SaEventAsset{};
        }

        AZ_Error("AudioEventAssetHandler", false, "The type requested is not supported.");
        return nullptr;
    }

    auto SaEventAssetHandler::LoadAssetData(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset,
        AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
        AZ::Data::AssetFilterCB const& assetLoadFilterCB) -> AZ::Data::AssetHandler::LoadResult
    {
        auto* assetData = asset.GetAs<SaEventAsset>();
        AZ_Assert(assetData, "Asset is of the wrong type.");
        AZ_Assert(m_serializeContext, "Cached SerializeContext pointer is null!") if (assetData)
        {
            return AZ::Utils::LoadObjectFromStreamInPlace<SaEventAsset>(
                       *stream,
                       *assetData,
                       m_serializeContext,
                       AZ::ObjectStream::FilterDescriptor(assetLoadFilterCB))
                ? AZ::Data::AssetHandler::LoadResult::LoadComplete
                : AZ::Data::AssetHandler::LoadResult::Error;
        }

        return AZ::Data::AssetHandler::LoadResult::Error;
    }

    auto SaEventAssetHandler::SaveAssetData(
        const AZ::Data::Asset<AZ::Data::AssetData>& asset, AZ::IO::GenericStream* stream) -> bool
    {
        auto* assetData = asset.GetAs<SaEventAsset>();
        AZ_Assert(assetData, "Asset is of the wrong type.");
        AZ_Assert(
            m_serializeContext,
            "Cached SerializeContext pointer is null!") if (assetData && m_serializeContext)
        {
            return AZ::Utils::SaveObjectToStream<SaEventAsset>(
                *stream, AZ::ObjectStream::ST_JSON, assetData, m_serializeContext);
        }

        return false;
    }

    void SaEventAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
    {
        delete ptr;
    }

    void SaEventAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes)
    {
        assetTypes.push_back(AZ::Data::AssetType{ SaEventAssetTypeId });
    }

    auto SaEventAssetHandler::GetAssetType() const -> AZ::Data::AssetType
    {
        return AZ::Data::AssetType{ SaEventAssetTypeId };
    }

    void SaEventAssetHandler::GetAssetTypeExtensions(AZStd::vector<AZStd::string>& extensions)
    {
        extensions.push_back(SaEventAsset::ProductExtension);
        extensions.push_back(".saevent");
    }

    auto SaEventAssetHandler::GetAssetTypeDisplayName() const -> char const*
    {
        return "Audio Event Asset";
    }

    auto SaEventAssetHandler::GetBrowserIcon() const -> char const*
    {
        return {};
    }

    auto SaEventAssetHandler::GetGroup() const -> char const*
    {
        return SaEventAsset::AssetGroup;
    }

    auto SaEventAssetHandler::GetComponentTypeId() const -> AZ::Uuid
    {
        return {};
    }

    auto SaEventAssetHandler::CanCreateComponent(AZ::Data::AssetId const& /*assetId*/) const -> bool
    {
        return false;
    }

}  // namespace SteamAudio
