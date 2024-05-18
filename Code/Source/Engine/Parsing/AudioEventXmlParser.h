#pragma once

#include "AzCore/XML/rapidxml.h"
#include "Engine/AudioEventAsset.h"

namespace SteamAudio
{
    class AudioEventXmlParser
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(AudioEventXmlParser);
        explicit AudioEventXmlParser(AZStd::unique_ptr<AZ::rapidxml::xml_document<char>> doc)
            : m_doc{ AZStd::move(doc) }
        {
        }

        void WriteIntoAsset(SaEventAsset* asset);

    protected:
        void Process(SaEventAsset* asset);

    private:
        AZStd::unique_ptr<AZ::rapidxml::xml_document<char>> m_doc{};
        bool m_processed{};
    };
}  // namespace SteamAudio
