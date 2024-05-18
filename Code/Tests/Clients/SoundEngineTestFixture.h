#pragma once

#include "AzCore/UnitTest/UnitTest.h"
#include "Clients/BaseTestFixture.h"
#include "IAudioInterfacesCommonData.h"

class SoundEngineTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();

        auto& audioImpl{ HostAudioSystemImpl() };
        AZ_TEST_START_TRACE_SUPPRESSION;
        ASSERT_EQ(audioImpl.Initialize(), Audio::EAudioRequestStatus::Failure);
        AZ_TEST_STOP_TRACE_SUPPRESSION(1);  // AudioEngine not available
    }

    void TearDown() override
    {
        BaseTestFixture::TearDown();
    }
};
