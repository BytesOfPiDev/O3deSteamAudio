#include "Engine/ResourceManager.h"

#include "AzCore/IO/Path/Path.h"
#include "AzCore/Module/Environment.h"
#include "Engine/ISoundEngine.h"
#include "Engine/SoundAsset.h"

#include "IAudioInterfacesCommonData.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        SoundResourceManager, "SoundResourceManager", "70FA87D5-FBF2-480A-AC8A-66BB91618479");

    static AZ::EnvironmentVariable<ma_resource_manager*> s_maResMgr;

    void CopySound(ma_sound const* fromSound, ma_sound* toSound)
    {
        if (!fromSound || !toSound)
        {
            return;
        }

        auto* const maEngine{ Util::GetMaEngine() };
        (maEngine != nullptr) && ma_sound_init_copy(maEngine, fromSound, 0, nullptr, toSound);
    }

    void CreateSound(AZStd::string_view soundName, ma_sound* sound)
    {
        auto* const maEngine{ Util::GetMaEngine() };
        ma_sound_init_from_file(maEngine, soundName.data(), 0, nullptr, nullptr, sound);
    }

    auto PathToSoundName(AZStd::string_view filePath) -> AZStd::string
    {
        return AZ::IO::PathView{ filePath }.Stem().String();
    }

    SoundResourceManager::SoundResourceManager()
    {
        auto resMgr{ AZ::Environment::CreateVariable<ma_resource_manager*>(s_maResMgrEnvName) };

        AZ_Verify(resMgr.Get() == nullptr, "A resource manager already exists!");
        if (!resMgr.Get())
        {
            return;
        }

        resMgr.Get() = aznew ma_resource_manager;

        SoundResourceManagerRequestBus::Handler::BusConnect();
    }

    SoundResourceManager::~SoundResourceManager()
    {
        SoundResourceManagerRequestBus::Handler::BusDisconnect();
        auto* resMgr{ s_maResMgr.Get() };
        s_maResMgr.Reset();

        AZStd::ranges::for_each(
            m_registeredNames,
            [resMgr](AZ::Name const& soundName)
            {
                auto const result{ ma_resource_manager_unregister_file(
                    resMgr, soundName.GetCStr()) };

                AZ_Error(
                    TYPEINFO_Name(),
                    result != MA_SUCCESS,
                    "Failed to unregister sound we registered during destruction. Error: %i.",
                    result);
            });

        delete resMgr;
        resMgr = nullptr;
    }

    void SoundResourceManager::Update()
    {
    }

    auto SoundResourceManager::RegisterSound(SaSoundAsset* soundData, AZ::Name soundName) -> bool
    {
        AZ_Info(TYPEINFO_Name(), "Registering %s with the resource manager.", soundName.GetCStr());

        static constexpr auto registerEncodedData = [](SaSoundAsset const* soundData,
                                                       AZStd::string_view soundName) -> ma_result
        {
            return ma_resource_manager_register_encoded_data(
                AZ::Interface<ma_resource_manager>::Get(),
                soundName.data(),
                soundData->GetBuffer().data(),
                soundData->GetBuffer().size());
        };

        static constexpr auto registerDecodedData =
            [](SaSoundAsset const* soundData, AZStd::string_view soundName)
        {
            return ma_resource_manager_register_decoded_data(
                AZ::Interface<ma_resource_manager>::Get(),
                soundName.data(),
                soundData->GetBuffer().data(),
                soundData->GetFrameCount(),
                ma_format_f32,
                soundData->GetChannelCount(),
                soundData->GetSampleRate());
        };

        if (soundData->GetSourceType() == Audio::AudioInputSourceType::WavFile &&
            !registerEncodedData(soundData, soundName.GetStringView()))
        {
            return false;
        }

        if (soundData->GetSourceType() == Audio::AudioInputSourceType::PcmFile &&
            registerDecodedData(soundData, soundName.GetStringView()))
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

        auto const result = ma_resource_manager_unregister_data(
            AZ::Interface<ma_resource_manager>::Get(), soundName.GetCStr());

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

}  // namespace SteamAudio
