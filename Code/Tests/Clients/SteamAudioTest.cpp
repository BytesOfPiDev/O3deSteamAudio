
#include <AzTest/AzTest.h>

#include "Clients/EngineTestFixture.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"
#include "IAudioSystemImplementation.h"

TEST_F(EngineTestFixture, SANITY_CHECK)
{
    EXPECT_TRUE(AZ::Interface<Audio::IAudioSystem>::Get() != nullptr);
}

TEST_F(
    EngineTestFixture,
    AudioImplExistsButIsNotInitAndSoundEngineDoesNotExist_CallInitializeOnImpl_ReturnsFailure)
{
    SteamAudio::AudioSystemImpl_steamaudio impl{ "test" };
    EXPECT_EQ(impl.Initialize(), Audio::EAudioRequestStatus::Failure);
}
