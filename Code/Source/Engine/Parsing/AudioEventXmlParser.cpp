#include "Engine/Parsing/AudioEventXmlParser.h"

#include "AzCore/StringFunc/StringFunc.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/Common_steamaudio.h"

namespace SteamAudio
{

    AZ_TYPE_INFO_WITH_NAME_IMPL(
        AudioEventXmlParser, "Audio Event Xml Parser", "7D028489-DA5D-48B5-A457-F6B28A3AAE9C");

    void AudioEventXmlParser::WriteIntoAsset(SaEventAsset* asset)
    {
        if (asset == nullptr)
        {
            AZ_Error(TYPEINFO_Name(), false, "Invalid asset");
            return;
        }

        Process(asset);
    }

    void AudioEventXmlParser::Process(SaEventAsset* asset)
    {
        if (m_processed)
        {
            return;
        }

        m_processed = true;

        auto const* const rootNode{ m_doc->first_node() };
        auto const* const eventRoot{ rootNode->first_node(AudioStrings::EventTag) };

        if (eventRoot == nullptr)
        {
            AZ_Error(TYPEINFO_Name(), false, "Failed to find node '%s'", AudioStrings::EventTag);
            return;
        }

        [eventRoot, asset]()
        {
            auto const* const eventNameAttrib =
                eventRoot->first_attribute(AudioStrings::NameAttribute);

            asset->SetEventName(eventNameAttrib != nullptr ? eventNameAttrib->value() : "");
        }();

        for (AZ::rapidxml::xml_node<char> const* child = eventRoot->first_node(); child != nullptr;
             child = child->next_sibling())
        {
            if (AZ::StringFunc::Equal(child->name(), AudioStrings::SoundAttribute))
            {
            }
        }
    }
}  // namespace SteamAudio
