#include <AzTest/AzTest.h>

#include "AzCore/XML/rapidxml.h"
#include "Engine/Parsing/AudioEventXmlParser.h"
#include "Tools/BufferConstants.h"
#include "Tools/Mocks/MockAudioEventAsset.h"

using ::testing::Return;

TEST(TestEnvironment, SanityCheck)
{
}

TEST(
    AudioEventAssetParsingTests,
    ParseValidXmlFileWithTwoValidTasks_WriteIntoAsset_AddTaskCalledTwice)
{
    MockAudioEventAsset mockAsset{};

    auto buffer = AZStd::string(ExampleEventXmlBuffer.data());
    EXPECT_EQ(buffer.size(), ExampleEventXmlBuffer.size());

    auto doc{ AZStd::make_unique<AZ::rapidxml::xml_document<char>>() };
    EXPECT_EQ(doc->parse<0>(buffer.data()), true);

    SteamAudio::AudioEventXmlParser taskBuilder{ AZStd::move(doc) };
    doc = nullptr;

    taskBuilder.WriteIntoAsset(&mockAsset);
}
