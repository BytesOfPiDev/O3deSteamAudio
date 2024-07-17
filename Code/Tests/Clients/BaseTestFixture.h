#pragma once

#include <AzCore/UnitTest/UnitTest.h>
#include <AzTest/AzTest.h>

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/Name/NameDictionary.h"
#include "AzCore/std/optional.h"
#include "AzFramework/IO/LocalFileIO.h"
#include "Engine/AudioEventAssetHandler.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/ISoundEngine.h"
#include "Engine/MaSoundEngine.h"
#include "Engine/SoundAssetHandler.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystemImplementation.h"

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
    AZ::NameDictionary m_dictionary{};
    AZStd::unique_ptr<AZ::ComponentApplication> m_app{};
    AZ::Entity* m_systemEntity{};
    AZ::IO::FileIOBase* m_prevFileIo{};
    AZStd::unique_ptr<AZ::IO::LocalFileIO> m_fileIo{};

    SteamAudio::SaEventAssetHandler m_audioEventAssetHandler{};
    SteamAudio::SaSoundAssetHandler m_soundAssetHandler{};
};
