
#pragma once

namespace SteamAudio
{
    // System Component TypeIds
    inline constexpr char const* SteamAudioSystemComponentTypeId =
        "{77C81C2D-63CC-4C14-B46D-870C7451F832}";
    inline constexpr char const* SteamAudioEditorSystemComponentTypeId =
        "{9D4EF797-4B63-42A1-925A-34C2FA440CF9}";

    // Module derived classes TypeIds
    inline constexpr char const* SteamAudioModuleInterfaceTypeId =
        "{24982942-2E35-4B25-887C-3296ACA00820}";
    inline constexpr char const* SteamAudioModuleTypeId = "{12B630A2-E7B1-4F70-A874-90E85FE4ED40}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr char const* SteamAudioEditorModuleTypeId = SteamAudioModuleTypeId;

    // Interface TypeIds
    inline constexpr char const* SteamAudioRequestsTypeId =
        "{37C2EE92-B61E-4317-BB37-D0AF17AB7A88}";

    constexpr auto SaEventAssetTypeId{ "{42A942C7-E634-4D1C-A4D2-DB894A035230}" };
    constexpr auto SaSoundAssetTypeId{ "C94B5C4C-EBCA-4203-9FE4-EFCFCE707D28" };
    constexpr auto SaSoundAssetHandlerTypeId{ "C03D4F81-5249-4097-B38D-41C6AFAC75F7" };
    constexpr auto SaEventAssetHandlerTypeId{ "{AD28F187-2EA4-465E-BB3E-6854696CB135}" };
}  // namespace SteamAudio
