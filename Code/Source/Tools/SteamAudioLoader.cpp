#include "Tools/SteamAudioLoader.h"

#include <AzCore/JSON/document.h>

#include "AudioFileUtils.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/Path/Path.h"

#include "Engine/Configuration.h"
#include "IAudioSystemControl.h"
#include "IAudioSystemEditor.h"

#include "Engine/Common_steamaudio.h"
#include "Tools/AudioSystemControl_steamaudio.h"
#include "Tools/AudioSystemEditor_steamaudio.h"

namespace SteamAudio
{
    void SteamAudioLoader::Load(AudioControls::IAudioSystemEditor* audioSystemEditor)
    {
        m_audioSystemEditor = audioSystemEditor;

        auto* const fileIo{ AZ::IO::FileIOBase::GetInstance() };

        auto const eventsFolder = [&fileIo]() -> AZ::IO::FixedMaxPath
        {
            AZStd::optional<AZ::IO::FixedMaxPath> const result{ fileIo->ResolvePath(EventsAlias) };

            return result.value_or("");
        }();

        auto const banksFolder = [&fileIo]() -> AZ::IO::FixedMaxPath
        {
            AZStd::optional<AZ::IO::FixedMaxPath> const result{ fileIo->ResolvePath(RuntimePath) };

            return result.value_or("");
        }();

        AZLOG_INFO("SteamAudioLoader is loading event controls from '%s'...", eventsFolder.c_str());
        LoadControlsInFolder(eventsFolder.String());

        LoadSoundBanks(banksFolder.String(), AudioStrings::BanksFolder, false);
    }

    void SteamAudioLoader::LoadControlsInFolder(AZStd::string_view const folderPath)
    {
        if (folderPath.empty())
        {
            AZ_Error("SteamAudioLoader", false, "Folder path is empty. Unable to load controls.");

            return;
        }

        auto* const fileIo{ AZ::IO::FileIOBase::GetInstance() };
        if (!fileIo)
        {
            AZ_Error(
                "SteamAudioLoader",
                false,
                "Failed to get the FileIO instance. Unable to load controls.");

            return;
        }

        auto const resolvedPath = fileIo->ResolvePath(folderPath);
        auto const foundFiles = Audio::FindFilesInPath(resolvedPath->c_str(), "*");
        for (auto const& filePath : foundFiles)
        {
            AZ_Assert(
                AZ::IO::FileIOBase::GetInstance()->Exists(filePath.c_str()),
                "FindFiles found file '%s' but FileIO says it doesn't exist!",
                filePath.c_str());

            if (AZ::IO::FileIOBase::GetInstance()->IsDirectory(filePath.c_str()))
            {
                LoadControlsInFolder(filePath.Native());
            }
            else
            {
                AZ_Info("SteamAudioLoader", "Loading Xml from '%s'.", filePath.c_str());

                Audio::ScopedXmlLoader xmlFileLoader(filePath.Native());
                if (!xmlFileLoader.HasError())
                {
                    LoadControl(xmlFileLoader.GetRootNode());
                }
            }
        }
    }

    void SteamAudioLoader::LoadControl(AZ::rapidxml::xml_node<char> const* xmlNode)
    {
        if (!xmlNode)
        {
            return;
        }

        ExtractControlsFromXML(
            xmlNode,
            SteamAudioControlType::Event,
            AudioStrings::EventTag,
            AudioStrings::NameAttribute);
        ExtractControlsFromXML(
            xmlNode,
            SteamAudioControlType::AuxBus,
            AudioStrings::AuxBusTag,
            AudioStrings::NameAttribute);

        ExtractControlsFromXML(
            xmlNode,
            SteamAudioControlType::SoundBank,
            AudioStrings::SoundBankTag,
            AudioStrings::NameAttribute);

        AZStd::string_view const xmlTag(xmlNode->name());
        bool const isSwitchTag{ xmlTag == AudioStrings::SwitchGroupTag };
        bool const isStateTag{ xmlTag == AudioStrings::StateGroupTag };

        if (isSwitchTag || isStateTag)
        {
            if (auto const nameAttr = xmlNode->first_attribute(AudioStrings::NameAttribute))
            {
                AZStd::string const parentName{ nameAttr->value() };
                AudioControls::IAudioSystemControl* group =
                    static_cast<AudioSystemEditor_steamaudio*>(m_audioSystemEditor)
                        ->GetControlByName(parentName);

                if (!group)
                {
                    group = m_audioSystemEditor->CreateControl(AudioControls::SControlDef(
                        parentName,
                        isSwitchTag ? SteamAudioControlType::SwitchGroup
                                    : SteamAudioControlType::GameStateGroup));
                }

                auto const childrenNode = xmlNode->first_node(AudioStrings::ChildrenListTag);
                if (childrenNode)
                {
                    auto childNode = childrenNode->first_node();
                    while (childNode)
                    {
                        if (auto childNameAttr =
                                childNode->first_attribute(AudioStrings::NameAttribute))
                        {
                            m_audioSystemEditor->CreateControl(AudioControls::SControlDef(
                                childNameAttr->value(),
                                isSwitchTag ? SteamAudioControlType::Switch
                                            : SteamAudioControlType::GameState,
                                false,
                                group));
                        }
                        childNode = childNode->next_sibling();
                    }
                }
            }
        }

        auto childNode = xmlNode->first_node();
        while (childNode)
        {
            LoadControl(childNode);
            childNode = childNode->next_sibling();
        }
    }

    void SteamAudioLoader::LoadSoundBanks(
        AZStd::string const& rootFolder, AZStd::string const& subPath, bool isLocalized)
    {
        auto const searchPath = [&]()
        {
            AZ::IO::FixedMaxPath result(rootFolder);
            result /= subPath;
            return result;
        }();

        AZ_Info("SteamAudioLoader", "Checking soundbanks in '%s'...", searchPath.c_str());

        auto foundFiles = Audio::FindFilesInPath(searchPath.Native(), "*");

        AZ_Info("SteamAudioLoader", "Found '%i' files.", foundFiles.size());

        bool isLocalizedLoaded = isLocalized;

        for (auto const& filePath : foundFiles)
        {
            AZLOG_INFO("Loading soundbank file '%s'.", filePath.c_str());

            AZ_Assert(
                AZ::IO::FileIOBase::GetInstance()->Exists(filePath.c_str()),
                "FindFiles found file '%s' but FileIO says it doesn't exist!",
                filePath.c_str());

            AZ::IO::PathView const fileName = filePath.Filename();

            if (AZ::IO::FileIOBase::GetInstance()->IsDirectory(filePath.c_str()))
            {
                if (fileName != ExternalSourcesPath && !isLocalizedLoaded)
                {
                    // each sub-folder represents a different language,
                    // we load only one as all of them should have the
                    // same content (in the future we want to have a
                    // consistency report to highlight if this is not the case)
                    m_localizationFolder.assign(fileName.Native().data(), fileName.Native().size());
                    LoadSoundBanks({ searchPath.Native() }, m_localizationFolder, true);
                    isLocalizedLoaded = true;
                }
            }
            else if (fileName.Extension() == SoundbankExtension && fileName != InitBank)
            {
                m_audioSystemEditor->CreateControl(AudioControls::SControlDef(
                    AZStd::string{ fileName.Native() },
                    SteamAudioControlType::SoundBank,
                    isLocalized,
                    nullptr,
                    { subPath }));
            }
        }
    }

    void SteamAudioLoader::ExtractControlsFromXML(
        AZ::rapidxml::xml_node<char> const* xmlNode,
        SteamAudioControlType type,
        AZStd::string_view const controlTag,
        AZStd::string_view const controlNameAttribute)
    {
        AZStd::string_view xmlTag(xmlNode->name());
        if (xmlTag == controlTag)
        {
            if (auto nameAttr = xmlNode->first_attribute(controlNameAttribute.data()))
            {
                auto* saControl =
                    static_cast<AudioSystemControl_steamaudio*>(m_audioSystemEditor->CreateControl(
                        AudioControls::SControlDef(nameAttr->value(), type)));

                if (auto soundFileAttr = xmlNode->first_attribute(AudioStrings::SoundFile))
                {
                    saControl->SetSoundFile(soundFileAttr->value());
                }
            }
        }
    }

    auto SteamAudioLoader::GetLocalizationFolder() const -> AZ::IO::PathView
    {
        return AZ::IO::PathView{ m_localizationFolder };
    }

} // namespace SteamAudio
