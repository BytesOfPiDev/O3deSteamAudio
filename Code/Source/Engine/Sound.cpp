#include "Engine/Sound.h"

#include "AzCore/PlatformDef.h"
#include "AzCore/std/typetraits/type_id.h"
#include "Engine/SaSoundAsset.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    SoundInstance::SoundInstance(AZ::Name soundName, bool loop, float volume)
        : m_soundName{ AZStd::move(soundName) }
    {
        m_lowLevelSound = AZStd::make_any<ma_sound>();

        auto& soundRef{ AZStd::any_cast<ma_sound&>(m_lowLevelSound) };
        ma_sound_set_volume(&soundRef, volume);
        ma_sound_set_looping(&soundRef, loop);

        auto const initResult{ ma_sound_init_from_file(
            Util::GetMaEngine(), m_soundName.GetCStr(), 0, nullptr, nullptr, &soundRef) };

        AZ_Error(
            AZ_FUNCTION_SIGNATURE,
            initResult == MA_SUCCESS,
            "SoundInstance setup failed - sound init unsuccessful");

        if (initResult != MA_SUCCESS)
        {
            return;
        }

        ma_sound_start(&soundRef);
    }

    SoundInstance::~SoundInstance()
    {
        auto& soundRef{ AZStd::any_cast<ma_sound&>(m_lowLevelSound) };
        ma_sound_stop(&soundRef);
        ma_sound_uninit(&soundRef);
    }

    SoundInstance::SoundInstance(SoundInstance&& other)
        : m_soundName{ AZStd::move(other.m_soundName) }
        , m_lowLevelSound(AZStd::move(other.m_lowLevelSound))
    {
        other.m_soundName = AZ::Name{};
        other.m_lowLevelSound = {};
    }

    SoundSource::SoundSource(AZ::Data::Asset<SaSoundAsset> soundAsset, AZ::Name name)
        : m_asset{ AZStd::move(soundAsset) }
        , m_name{ AZStd::move(name) }
    {
        m_asset.QueueLoad() ? m_asset.BlockUntilLoadComplete() : AZ::Data::AssetData::AssetStatus();

        AZ_Error("SoundSource", m_asset, "Asset has no data!");
        AZ_Error(
            "SoundSource", m_asset.IsReady(), "Asset is not ready! Sound will not be registered.");
        AZ_Error(
            "SoundSource",
            aztypeid_cmp(m_asset.GetType(), AZ::AzTypeInfo<SaSoundAsset>::Uuid()),
            "The provided asset is not a sound asset!");
        AZ_Error(
            "SoundSource", !m_name.IsEmpty(), "Empty name provided! Sound will not be registered.");

        if (!m_asset.IsReady() || m_name.IsEmpty())
        {
            AZ_Error("SoundSource", false, "Asset isn't ready or name is empty");
            return;
        }

        Register();
    }

    SoundSource::~SoundSource()
    {
        if (!m_asset || m_name.IsEmpty())
        {
            return;
        }

        auto const result = ma_resource_manager_unregister_data(
            ma_engine_get_resource_manager(Util::GetMaEngine()), m_name.GetCStr());

        AZ_Error(
            "SoundSource",
            result == MA_SUCCESS,
            "Failed to unregister asset '%s' with name '%s'",
            m_asset.GetHint().c_str(),
            m_name.GetCStr());
    }

    void SoundSource::Register()
    {
        auto const* const soundAssetData{ m_asset.GetAs<SaSoundAsset>() };

        AZ_Error(
            AZ_FUNCTION_SIGNATURE,
            soundAssetData,
            "No asset data. Unable to register sound source.");

        if (!soundAssetData)
        {
            return;
        }

        auto const result = ma_resource_manager_register_encoded_data(
            ma_engine_get_resource_manager(Util::GetMaEngine()),
            m_name.GetCStr(),
            soundAssetData->m_data.data(),
            soundAssetData->m_data.size());

        AZ_Error(
            AZ_FUNCTION_SIGNATURE,
            result == MA_SUCCESS,
            "Failed to register sound asset '%s' with the miniaudio resource manager",
            m_name.GetCStr());

        if (result != MA_SUCCESS)
        {
            m_asset.Reset();
            m_name = AZ::Name{};
        }
    }
}  // namespace SteamAudio
