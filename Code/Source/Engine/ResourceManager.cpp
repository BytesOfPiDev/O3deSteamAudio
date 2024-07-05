#include "Engine/ResourceManager.h"

#include "Engine/SoundAsset.h"

#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        SoundResourceManager, "SoundResourceManager", "70FA87D5-FBF2-480A-AC8A-66BB91618479");

    static auto GetResourceManager() -> ma_resource_manager*
    {
        static ma_resource_manager instance{};
        static auto const result{ []() -> ma_result
                                  {
                                      ma_resource_manager_config config;
                                      return ma_resource_manager_init(&config, &instance);
                                  }() };

        AZ_ErrorOnce(
            SoundResourceManager::TYPEINFO_Name(),
            result != MA_SUCCESS,
            "The miniaudio resource manager failed to initialize!");

        return &instance;
    }

    void SoundResourceManager::Update()
    {
    }

    auto SoundResourceManager::RegisterSound(SaSoundAsset* soundData, AZ::Name soundName) -> bool
    {
        AZ_Info(TYPEINFO_Name(), "Registering %s with the resource manager.", soundName.GetCStr());

        auto const result{ ma_resource_manager_register_decoded_data(
            GetResourceManager(),
            soundName.GetCStr(),
            soundData->GetBuffer().data(),
            soundData->GetFrameCount(),
            ma_format_f32,
            soundData->GetChannelCount(),
            soundData->GetSampleRate()) };

        if (result != MA_SUCCESS)
        {
            return false;
        }

        m_registeredNames.insert(AZStd::move(soundName));
        return true;
    }

    auto SoundResourceManager::UnregisterSound(AZ::Name soundName) -> bool
    {
        // We only unregister names we've previously registered.
        if (!m_registeredNames.contains(soundName))
        {
            AZ_Warning(
                TYPEINFO_Name(),
                false,
                "Attempting to unregister sound we did not register. Refusing.");

            return false;
        }

        auto const result =
            ma_resource_manager_unregister_data(GetResourceManager(), soundName.GetCStr());

        if (result != MA_SUCCESS)
        {
            AZ_Error(TYPEINFO_Name(), false, "Failed to unregister sound. Error: %i", result);
            return false;
        }

        return result == MA_SUCCESS;
    }

    auto SoundResourceManager::CreateSound(AZ::Name) -> AZ::Outcome<Sound, AZStd::string>
    {
        return AZ::Failure("Unimplemented");
    }

    SoundResourceManager::SoundResourceManager()
    {
        SoundResourceManagerRequestBus::Handler::BusConnect();
    }

    SoundResourceManager::~SoundResourceManager()
    {
        SoundResourceManagerRequestBus::Handler::BusDisconnect();

        AZStd::ranges::for_each(
            m_registeredNames,
            [](AZ::Name const& soundName)
            {
                auto const result{ ma_resource_manager_unregister_file(
                    GetResourceManager(), soundName.GetCStr()) };

                AZ_Error(
                    TYPEINFO_Name(),
                    result != MA_SUCCESS,
                    "Failed to unregister sound we registered during destruction. Error: %i.",
                    result);
            });
    }
}  // namespace SteamAudio
