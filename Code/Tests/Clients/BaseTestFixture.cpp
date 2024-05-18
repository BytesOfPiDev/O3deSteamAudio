#include "Clients/BaseTestFixture.h"

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Settings/SettingsRegistry.h"
#include "AzCore/Utils/Utils.h"

void BaseTestFixture::SetUp()
{
    m_prevFileIo = AZ::IO::FileIOBase::GetInstance();
    AZ::IO::FileIOBase::SetInstance(nullptr);

    m_fileIo = AZStd::make_unique<AZ::IO::LocalFileIO>();
    AZ::IO::FileIOBase::SetInstance(m_fileIo.get());

    AZ::ComponentApplication::Descriptor appDesc;
    AZ::ComponentApplication::StartupParameters startupParams;

    startupParams.m_loadSettingsRegistry = true;
    m_systemEntity = m_app.Create(appDesc, startupParams);

    m_systemEntity->Init();
    m_systemEntity->Activate();

    AZ::Test::AddActiveGem("SteamAudio", *AZ::SettingsRegistry::Get(), GetFileIo());
    ASSERT_TRUE(m_fileIo->ResolvePath("@gemroot:SteamAudio@").has_value());

    AZStd::optional<AZ::IO::FixedMaxPath> const productPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Products");
    ASSERT_TRUE(productPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@products@", productPath.value().c_str());

    AZStd::optional<AZ::IO::FixedMaxPath> const testAssetPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Assets");
    ASSERT_TRUE(testAssetPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@assets@", testAssetPath.value().c_str());
}

void BaseTestFixture::TearDown()
{
    if (m_audioSystemImpl.has_value())
    {
        m_audioSystemImpl.reset();
    }

    if (m_soundEngine.has_value())
    {
        m_soundEngine.reset();
    }

    m_systemEntity = nullptr;
    m_app.Destroy();

    AZ::IO::FileIOBase::SetInstance(nullptr);
    m_fileIo = nullptr;

    AZ::IO::FileIOBase::SetInstance(m_prevFileIo);
    m_prevFileIo = nullptr;
}
