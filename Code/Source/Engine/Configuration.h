#pragma once

#include "AzCore/IO/Path/Path.h"

namespace SteamAudio
{
    namespace Settings
    {
        // Settings Registry Keys
        static constexpr auto AudioChannelsKey{ "SteamAudio/Config/AudioChannels" };
        static constexpr auto SampleRateKey{ "SteamAudio/Config/SampleRate" };
        static constexpr auto BitsPerSampleKey{ "SteamAudio/Config/SampleRate" };
        static constexpr auto LanguageKey{ "SteamAudio/Config/Language" };
    }  // namespace Settings

    static constexpr auto BanksAlias = "@soundbanks@";
    static constexpr auto SoundsAlias = "@soundbanks@";
    static constexpr auto EventsAlias = "@audioevents@";

    static constexpr auto BasePath = "@audioproject@";

    static constexpr auto ExternalSourcesPath = "external";
    static constexpr auto SoundbankExtension = "sabank";
    static constexpr auto SoundbankExtensionWildcard = "*.sabank";
    static constexpr auto SoundbankDependencyFileExtension{ ".soundbankdeps" };
    static constexpr auto InitBank = "init.sabank";

    auto GetBanksRootPath() -> AZ::IO::PathView;

    static constexpr auto DefaultAudioChannels = 2;
    static constexpr auto DefaultSampleRate = 48000;
    static constexpr auto DefaultBitsPerSample = 16;
    static constexpr auto DefaultFrameSize = 1024;

    static constexpr auto StopAllSoundsEventName{ "StopAllSounds" };

    AZ_ENUM_CLASS(Language, en, es);

    class ConfigurationSettings
    {
    public:
        auto Load(AZ::IO::PathView configFile) -> bool;

        [[nodiscard]] auto GetLanguage() const -> Language
        {
            return m_language;
        }

    private:
        Language m_language{};
    };

}  // namespace SteamAudio
