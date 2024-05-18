#include "Engine/ResourceManager.h"

#include "Engine/SoundAsset.h"

#include "miniaudio.h"

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

    auto SoundResourceManager::RegisterSound(
        AZ::Data::Asset<SaSoundAsset> soundData, AZ::Name soundName) -> bool
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

        return result == MA_SUCCESS;
    }

    auto SoundResourceManager::UnregisterSound(AZ::Name soundName) -> bool
    {
        auto const result =
            ma_resource_manager_unregister_data(GetResourceManager(), soundName.GetCStr());

        return result == MA_SUCCESS;
    }

    auto SoundResourceManager::CreateSound(AZ::Name) -> AZ::Outcome<Sound, AZStd::string>
    {
        return AZ::Failure("Unimplemented");
    }
}  // namespace SteamAudio
