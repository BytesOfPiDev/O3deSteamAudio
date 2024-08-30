#include "Engine/Sound.h"

#include "AzCore/std/typetraits/type_id.h"
#include "Engine/SaSoundAsset.h"
#include "SteamAudio/MiniAudio.h"
#include "SteamAudio/Util.h"

namespace SteamAudio
{
    SoundInstance::SoundInstance(AZ::Data::Asset<SaSoundAsset> soundAsset)
        : m_soundAsset{ AZStd::move(soundAsset) }
    {
        m_lowLevelSound = AZStd::make_any<ma_sound>();
    }

    SoundSource::SoundSource(AZ::Data::Asset<AZ::Data::AssetData> asset)
        : m_asset{ AZStd::move(asset) }
    {
    }

    SoundSource::SoundSource(AZ::Data::Asset<AZ::Data::AssetData> soundAsset, AZ::Name name)
        : m_asset{ AZStd::move(soundAsset) }
        , m_name{ AZStd::move(name) }
    {
        m_asset.QueueLoad() ? m_asset.BlockUntilLoadComplete() : AZ::Data::AssetData::AssetStatus();

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
