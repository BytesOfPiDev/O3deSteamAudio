#include "Engine/SoundAssetHandler.h"

#include "AudioAllocators.h"
#include "AzCore/Component/ComponentApplicationBus.h"
#include "AzCore/IO/Path/Path_fwd.h"
#include "AzCore/Serialization/Utils.h"
#include "Engine/Configuration.h"
#include "Engine/ResourceManager.h"
#include "Engine/SoundAsset.h"
#include "Engine/SoundEngine.h"
#include "SteamAudio/SteamAudioTypeIds.h"
#include "SteamAudio/Util.h"

#include "miniaudio.h"

namespace SteamAudio
{
    AZ_CLASS_ALLOCATOR_IMPL(SaSoundAssetHandler, Audio::AudioImplAllocator);
    AZ_RTTI_NO_TYPE_INFO_IMPL(SaSoundAssetHandler, AZ::Data::AssetHandler);
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        SaSoundAssetHandler, "SaSoundAssetHandler", SaSoundAssetHandlerTypeId);

    class SaResourceManager : public SaSoundAssetHandler::IResourceManagerImpl
    {
    public:
        SaResourceManager();
        ~SaResourceManager();

        ma_resource_manager_config m_maResourceManagerConfig{};
        ma_resource_manager m_maResourceManager{};
    };

    SaResourceManager::SaResourceManager()
    {
        m_maResourceManagerConfig = ma_resource_manager_config_init();
        m_maResourceManagerConfig.decodedFormat = ma_format_f32;
        m_maResourceManagerConfig.decodedChannels = DefaultAudioChannels;
        m_maResourceManagerConfig.decodedSampleRate = DefaultSampleRate;

        auto const result =
            ma_resource_manager_init(&m_maResourceManagerConfig, &m_maResourceManager);

        AZ_Error(
            "SaResourceManager",
            result != MA_SUCCESS,
            "Failed to initialize miniaudio resource manager");
    }

    SaSoundAssetHandler::SaSoundAssetHandler()
        : m_impl(AZStd::make_unique<SaResourceManager>()){};

    SaSoundAssetHandler::~SaSoundAssetHandler() = default;

    void SteamAudio::SaSoundAssetHandler::InitAsset(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset,
        bool /*loadStageSucceeded*/,
        bool /*isReload*/)
    {
        AZ::Data::Asset<SaSoundAsset> soundAsset{ asset };
        auto const assetHint{ AZ::IO::Path{ asset.GetHint() } };

        AZ_Error(
            TYPEINFO_Name(),
            soundAsset->IsLoading(true),
            "Expected asset to be in the loading or queued to load state.");

        SoundResourceManagerRequestBus::Broadcast(
            &SoundResourceManagerRequests::RegisterSound,
            AZStd::move(soundAsset),
            AZ::Name{ assetHint.Stem().String() });

        soundAsset.Reset();

        AZ::Data::AssetManagerBus::Broadcast(
            &AZ::Data::AssetManagerBus::Events::OnAssetReady, asset);
    }

    auto SteamAudio::SaSoundAssetHandler::CreateAsset(
        const AZ::Data::AssetId& /*id*/, const AZ::Data::AssetType& type) -> AZ::Data::AssetPtr
    {
        if (type == AZ::Data::AssetType{ SaSoundAssetTypeId })
        {
            return aznew SaSoundAsset{};
        }

        AZ_Error("AudioEventAssetHandler", false, "The type requested is not supported.");
        return nullptr;
    }

    auto SaSoundAssetHandler::LoadAssetData(
        AZ::Data::Asset<AZ::Data::AssetData> const& asset,
        AZStd::shared_ptr<AZ::Data::AssetDataStream> stream,
        AZ::Data::AssetFilterCB const& assetLoadFilterCB) -> AZ::Data::AssetHandler::LoadResult
    {
        auto* assetData = asset.GetAs<SaSoundAsset>();
        AZ_Assert(assetData, "Asset is of the wrong type.");
        AZ_Assert(m_serializeContext, "Cached SerializeContext pointer is null!") if (assetData)
        {
            return AZ::Utils::LoadObjectFromStreamInPlace<SaSoundAsset>(
                       *stream,
                       *assetData,
                       m_serializeContext,
                       AZ::ObjectStream::FilterDescriptor(assetLoadFilterCB))
                ? AZ::Data::AssetHandler::LoadResult::LoadComplete
                : AZ::Data::AssetHandler::LoadResult::Error;
        }

        return AZ::Data::AssetHandler::LoadResult::Error;
    }

    auto SaSoundAssetHandler::SaveAssetData(
        const AZ::Data::Asset<AZ::Data::AssetData>& asset, AZ::IO::GenericStream* stream) -> bool
    {
        auto* assetData = asset.GetAs<SaSoundAsset>();
        AZ_Assert(assetData, "Asset is of the wrong type.");
        AZ_Assert(m_serializeContext, "Cached SerializeContext pointer is null!");

        if (assetData && m_serializeContext)
        {
            return AZ::Utils::SaveObjectToStream<SaSoundAsset>(
                *stream, AZ::ObjectStream::ST_JSON, assetData, m_serializeContext);
        }

        return false;
    }

    void SaSoundAssetHandler::DestroyAsset(AZ::Data::AssetPtr ptr)
    {
        delete ptr;
    }

    void SaSoundAssetHandler::GetHandledAssetTypes(AZStd::vector<AZ::Data::AssetType>& assetTypes)
    {
        assetTypes.push_back(AZ::Data::AssetType{ SaSoundAssetTypeId });
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
        return "SteamAudio Sound";
    }
    auto SaSoundAssetHandler::GetBrowserIcon() const -> char const*
    {
        return {};
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

    auto SaSoundAssetHandler::CanHandleAsset(AZ::Data::AssetId const& id) const -> bool
    {
        AZ::IO::Path const assetPath = Util::GetAssetPath(id);
        return assetPath.Match(SaSoundAsset::ProductExtensionWildcard);
    }
}  // namespace SteamAudio
