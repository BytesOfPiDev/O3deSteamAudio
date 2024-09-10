#pragma once

#include <AzCore/UnitTest/UnitTest.h>
#include <AzTest/AzTest.h>

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzFramework/IO/LocalFileIO.h"

#include "Engine/SaEventAsset.h"
#include "Engine/SoundAssetHandler.h"

class BaseTestFixture
    : public UnitTest::TraceBusRedirector
    , public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    auto GetFileIo() -> AZ::IO::FileIOBase*
    {
        return m_fileIo.get();
    }

private:
    AZStd::unique_ptr<AZ::ComponentApplication> m_app{};
    AZ::Entity* m_systemEntity{};
    AZ::IO::FileIOBase* m_prevFileIo{};
    AZStd::unique_ptr<AZ::IO::LocalFileIO> m_fileIo{};

    AZStd::unique_ptr<SteamAudio::SaEventAssetGenericHandler> m_audioEventAssetHandler{};
    AZStd::unique_ptr<SteamAudio::SaSoundAssetHandler> m_soundAssetHandler{};
};
