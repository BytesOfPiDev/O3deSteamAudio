//

#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Asset/AssetTypeInfoBus.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "IAudioSystem.h"
#include "IAudioSystemImplementation.h"
#include "ISystem.h"
#include "Mocks/IConsoleMock.h"
#include "Mocks/ISystemMock.h"

using ::testing::NiceMock;
class SteamAudioTestEnvironment : public UnitTest::TraceBusHook
{
public:
    AZ_DISABLE_COPY_MOVE(SteamAudioTestEnvironment);

    SteamAudioTestEnvironment() = default;
    ~SteamAudioTestEnvironment() override = default;

protected:
    struct MockHolder
    {
        NiceMock<ConsoleMock> m_console;
        NiceMock<SystemMock> m_system;
    };

    void SetupEnvironment() override
    {
        UnitTest::TraceBusHook::SetupEnvironment();

        m_mocks = new MockHolder{};
        m_stubEnv.pConsole = &m_mocks->m_console;
        m_stubEnv.pSystem = &m_mocks->m_system;
        gEnv = &m_stubEnv;

        AZ::Data::AssetCatalogRequestBus::GetOrCreateContext();
        AZ::Data::AssetManagerNotificationBus::GetOrCreateContext();
        AZ::AssetTypeInfoBus::GetOrCreateContext();
        Audio::AudioSystemImplementationRequestBus::GetOrCreateContext();
        Audio::Gem::EngineRequestBus::GetOrCreateContext();
    }

    void TeardownEnvironment() override
    {
        UnitTest::TraceBusHook::TeardownEnvironment();

        delete m_mocks;
        m_mocks = nullptr;
    }

private:
    SSystemGlobalEnvironment m_stubEnv{};
    MockHolder* m_mocks{ nullptr };
};

AZ_UNIT_TEST_HOOK(new SteamAudioTestEnvironment);