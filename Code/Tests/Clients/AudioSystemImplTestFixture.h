#pragma once

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/UnitTest/UnitTest.h"

class AudioSystemImplTestFixture
    : public UnitTest::TraceBusRedirector
    , public testing::Test
{
public:
    AZ_DISABLE_COPY_MOVE(AudioSystemImplTestFixture);

    AudioSystemImplTestFixture() = default;
    ~AudioSystemImplTestFixture() override = default;

    void SetUp() override;
    void TearDown() override;

private:
    AZStd::unique_ptr<AZ::ComponentApplication> m_app{};
};
