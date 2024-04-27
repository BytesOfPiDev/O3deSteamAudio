#pragma once

#include "AzCore/IO/Path/Path.h"
#include "AzCore/XML/rapidxml.h"
#include "Engine/Configuration.h"
#include "IAudioSystemEditor.h"

#include "Tools/AudioSystemControl_steamaudio.h"

namespace SteamAudio
{
    class SteamAudioLoader
    {
    public:
        SteamAudioLoader() = default;

        void Load(AudioControls::IAudioSystemEditor* audioSystemImpl);
        [[nodiscard]] auto GetLocalizationFolder() const -> AZ::IO::PathView;

    protected:
        void LoadControlsInFolder(AZStd::string_view const folderPath);
        void LoadControl(AZ::rapidxml::xml_node<char> const* xmlNode);
        void LoadSoundBanks(
            AZStd::string const& rootFolder, AZStd::string const& subPath, bool isLocalized);

        void ExtractControlsFromXML(
            const AZ::rapidxml::xml_node<char>* xmlNode,
            SteamAudioControlType type,
            AZStd::string_view const controlTag,
            AZStd::string_view const controlNameAttribute);

    private:
        AZStd::string m_localizationFolder;
        AudioControls::IAudioSystemEditor* m_audioSystemEditor = nullptr;
    };

} // namespace SteamAudio
