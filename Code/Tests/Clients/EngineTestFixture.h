#pragma once

#include "AzCore/UnitTest/UnitTest.h"
#include "AzCore/Component/ComponentApplication.h"

class EngineTestFixture
    : public UnitTest::TraceBusRedirector
    , public testing::Test
{
public:
    AZ_DISABLE_COPY_MOVE(EngineTestFixture);

    EngineTestFixture() = default;
    virtual ~EngineTestFixture() = default;
    
    void SetUp() override;
    void TearDown() override;
    
    private:
    AZStd::unique_ptr<AZ::ComponentApplication> m_app{};
    
};