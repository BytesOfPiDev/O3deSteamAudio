#include "Clients/BaseTestFixture.h"

#include "AzCore/Asset/AssetManagerComponent.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/Streamer/StreamerComponent.h"
#include "AzCore/Jobs/JobManagerComponent.h"
#include "AzCore/Settings/SettingsRegistry.h"

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

    m_systemEntity = m_app->Create(appDesc, startupParams);
    m_systemEntity->Init();
    m_systemEntity->Activate();

    AZ::Test::AddActiveGem("SteamAudio", *AZ::SettingsRegistry::Get(), GetFileIo());
    ASSERT_TRUE(m_fileIo->ResolvePath("@gemroot:SteamAudio@").has_value());

    AZStd::optional<AZ::IO::FixedMaxPath> const productPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Cache/test");
    ASSERT_TRUE(productPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@products@", productPath.value().c_str());

    AZStd::optional<AZ::IO::FixedMaxPath> const testAssetPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Assets");
    ASSERT_TRUE(testAssetPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@assets@", testAssetPath.value().c_str());

    m_audioEventAssetHandler.Register();
    m_soundAssetHandler.Register();
}

void BaseTestFixture::TearDown()
{
    m_soundAssetHandler.Unregister();
    m_audioEventAssetHandler.Unregister();

    m_systemEntity = nullptr;
    m_app->Destroy();
    m_app = nullptr;

    AZ::IO::FileIOBase::SetInstance(nullptr);
    m_fileIo = nullptr;

    AZ::IO::FileIOBase::SetInstance(m_prevFileIo);
    m_prevFileIo = nullptr;
}
