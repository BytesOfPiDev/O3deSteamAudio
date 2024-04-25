
#pragma once

namespace SteamAudio
{
    // System Component TypeIds
    inline constexpr const char* SteamAudioSystemComponentTypeId = "{77C81C2D-63CC-4C14-B46D-870C7451F832}";
    inline constexpr const char* SteamAudioEditorSystemComponentTypeId = "{9D4EF797-4B63-42A1-925A-34C2FA440CF9}";

    // Module derived classes TypeIds
    inline constexpr const char* SteamAudioModuleInterfaceTypeId = "{24982942-2E35-4B25-887C-3296ACA00820}";
    inline constexpr const char* SteamAudioModuleTypeId = "{12B630A2-E7B1-4F70-A874-90E85FE4ED40}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* SteamAudioEditorModuleTypeId = SteamAudioModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* SteamAudioRequestsTypeId = "{37C2EE92-B61E-4317-BB37-D0AF17AB7A88}";
} // namespace SteamAudio
