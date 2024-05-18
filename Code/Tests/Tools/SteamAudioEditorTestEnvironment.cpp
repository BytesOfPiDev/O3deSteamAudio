#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/base.h"
#include "AzFramework/IO/LocalFileIO.h"
#include "AzTest/GemTestEnvironment.h"
#include "ISystem.h"
#include "UnitTest/ToolsTestApplication.h"
#include "gmock/gmock-generated-nice-strict.h"

using ::testing::NiceMock;

class SaEditorApp : public UnitTest::ToolsTestApplication
{
public:
    AZ_DISABLE_COPY_MOVE(SaEditorApp);

    SaEditorApp()
        : UnitTest::ToolsTestApplication("SteamAudioTestApp")
    {
    }

    ~SaEditorApp() override = default;
};

class SteamAudioEditorTestEnvironment : public AZ::Test::GemTestEnvironment
{
public:
    AZ_DISABLE_COPY_MOVE(SteamAudioEditorTestEnvironment);

    SteamAudioEditorTestEnvironment() = default;
    ~SteamAudioEditorTestEnvironment() override = default;

    void AddGemsAndComponents() override;
    void SetupEnvironment() override;

    void TeardownEnvironment() override;

    auto CreateApplicationInstance() -> AZ::ComponentApplication* override;
};

void SteamAudioEditorTestEnvironment::AddGemsAndComponents()
{
    AddActiveGems(AZStd::to_array<AZStd::string_view>({ "SteamAudio" }));
}

void SteamAudioEditorTestEnvironment::SetupEnvironment()
{
    AZ::Test::GemTestEnvironment::SetupEnvironment();

    AZStd::optional<AZ::IO::FixedMaxPath> const rootPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test");
    ASSERT_TRUE(rootPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@root@", rootPath.value().c_str());

    AZStd::optional<AZ::IO::FixedMaxPath> const productsPath =
        AZ::IO::FileIOBase::GetInstance()->ResolvePath("@gemroot:SteamAudio@/Test/Cache");
    ASSERT_TRUE(productsPath.has_value());
    AZ::IO::FileIOBase::GetInstance()->SetAlias("@products@", productsPath.value().c_str());
}

void SteamAudioEditorTestEnvironment::TeardownEnvironment()
{
    AZ::Test::GemTestEnvironment::TeardownEnvironment();
}

auto SteamAudioEditorTestEnvironment::CreateApplicationInstance() -> AZ::ComponentApplication*
{
    return new SaEditorApp();
}

AZ_UNIT_TEST_HOOK(aznew SteamAudioEditorTestEnvironment);
