#include "Clients/BaseTestFixture.h"

#include "AzCore/Asset/AssetManagerComponent.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/Streamer/StreamerComponent.h"
#include "AzCore/Jobs/JobManagerComponent.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Settings/SettingsRegistry.h"
#include "AzFramework/Asset/AssetCatalogComponent.h"

#include "Engine/Configuration.h"
#include "Engine/SaEventAsset.h"
#include "Engine/SaEventAssetHandler.h"

class BaseApp : public AZ::ComponentApplication
{
public:
    auto GetRequiredSystemComponents() const -> AZ::ComponentTypeList override
    {
        auto required{ AZ::ComponentApplication::GetRequiredSystemComponents() };
        required.push_back(azrtti_typeid<AZ::StreamerComponent>());
        required.push_back(azrtti_typeid<AZ::JobManagerComponent>());
        required.push_back(azrtti_typeid<AZ::AssetManagerComponent>());
        required.push_back(azrtti_typeid<AZ::StreamerComponent>());
        required.push_back(azrtti_typeid<AZ::JobManagerComponent>());
        required.push_back(azrtti_typeid<AzFramework::AssetCatalogComponent>());

        return required;
    }
};

void BaseTestFixture::SetUp()
{
    m_prevFileIo = AZ::IO::FileIOBase::GetInstance();
    AZ::IO::FileIOBase::SetInstance(nullptr);

    m_fileIo = AZStd::make_unique<AZ::IO::LocalFileIO>();
    AZ::IO::FileIOBase::SetInstance(m_fileIo.get());

    AZ::ComponentApplication::Descriptor appDesc;
    AZ::ComponentApplication::StartupParameters startupParams;
    startupParams.m_loadSettingsRegistry = true;
    startupParams.m_loadAssetCatalog = true;

    m_app = AZStd::make_unique<BaseApp>();
    m_app->RegisterComponentDescriptor(AZ::AssetManagerComponent::CreateDescriptor());
    m_app->RegisterComponentDescriptor(AzFramework::AssetCatalogComponent::CreateDescriptor());

    m_systemEntity = m_app->Create(appDesc, startupParams);

    SteamAudio::SaEventAsset::Reflect(m_app->GetSerializeContext());
    m_systemEntity->Init();
    m_systemEntity->Activate();

    AZ::Test::AddActiveGem("SteamAudio", *AZ::SettingsRegistry::Get(), GetFileIo());
    ASSERT_TRUE(m_fileIo->ResolvePath("@gemroot:SteamAudio@").has_value());

    // FIXME: Dynamically set OS folder
    AZStd::optional<AZ::IO::FixedMaxPath> const productPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Cache/linux");
    ASSERT_TRUE(productPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@products@", productPath.value().c_str());

    AZStd::optional<AZ::IO::FixedMaxPath> const testAssetPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Assets");
    ASSERT_TRUE(testAssetPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@assets@", testAssetPath.value().c_str());

    AZStd::optional<AZ::IO::FixedMaxPath> const testEventsPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@products@/sounds/steamaudio/events");
    ASSERT_TRUE(testEventsPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias(
        SteamAudio::EventsAlias, testEventsPath.value().c_str());

    m_audioEventAssetHandler = AZStd::make_unique<SteamAudio::SaEventAssetHandler>();
    m_soundAssetHandler = AZStd::make_unique<SteamAudio::SaSoundAssetGenericHandler>(
        "SteamAudio Sound Asset",
        SteamAudio::SaSoundAsset::AssetGroup,
        SteamAudio::SaSoundAsset::ProductExtension);
    m_audioEventAssetHandler->Register();
    m_soundAssetHandler->Register();
}

void BaseTestFixture::TearDown()
{
    m_systemEntity = nullptr;
    m_app->Destroy();
    m_app = nullptr;

    [[maybe_unused]] auto* p1{ m_soundAssetHandler.release() };
    [[maybe_unused]] auto* p2{ m_audioEventAssetHandler.release() };

    AZ::IO::FileIOBase::SetInstance(nullptr);
    m_fileIo = nullptr;

    AZ::IO::FileIOBase::SetInstance(m_prevFileIo);
    m_prevFileIo = nullptr;
}
