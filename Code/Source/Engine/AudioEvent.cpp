#include "Engine/AudioEvent.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "Engine/AudioEventAsset.h"
#include "Engine/Id.h"

#include "AzCore/Console/ILogger.h"
#include "Engine/SoundAsset.h"
#include "SteamAudio/MiniAudio.h"

namespace SteamAudio
{

    struct PlaySoundFunc
    {
        PlaySoundFunc()
            : m_sound{}
        {
        }

        PlaySoundFunc(AZ::Data::Asset<SaSoundAsset> const& asset)
            : m_asset{ asset }
        {
            m_asset ? m_asset->CopySoundInto(&m_sound) : void();
        }

        PlaySoundFunc(PlaySoundFunc const& other)
            : m_asset{ other.m_asset }
        {
            other.m_asset ? other.m_asset->CopySoundInto(&m_sound) : void();
        }

        PlaySoundFunc(PlaySoundFunc&& other)
            : m_asset(AZStd::move(other.m_asset))
        {
            other.m_asset = {};
            ma_sound_uninit(&other.m_sound);

            m_asset ? m_asset->CopySoundInto(&m_sound) : void();
        }

        ~PlaySoundFunc() = default;

        auto operator=(PlaySoundFunc const& other) -> PlaySoundFunc&
        {
            m_asset = other.m_asset;
            ma_sound_uninit(&m_sound);
            other.m_asset->CopySoundInto(&m_sound);

            return *this;
        }
        auto operator=(PlaySoundFunc&& other) -> PlaySoundFunc&
        {
            ma_sound_uninit(&m_sound);
            ma_sound_uninit(&other.m_sound);

            m_asset = AZStd::move(other.m_asset);
            other.m_asset = {};

            m_asset ? m_asset->CopySoundInto(&m_sound) : void();

            return *this;
        }

        void operator()(SaGameObjectId)
        {
        }

        AZ::Data::Asset<SaSoundAsset> m_asset{};
        ma_sound m_sound{};
    };

    using DefaultTask = PlaySoundFunc;

    SaEvent::SaEvent() = default;

    SaEvent::SaEvent(AZ::Data::AssetId eventAssetId)
        : m_startFunc()
    {
        if (eventAssetId.IsValid())
        {
            AZ_Warning(__FUNCTION__, false, "Constructed with invalid id");
            return;
        }

        auto eventAsset{ AZ::Data::AssetManager::Instance().GetAsset<SaEventAsset>(
            eventAssetId, AZ::Data::AssetLoadBehavior::PreLoad) };
        eventAsset.BlockUntilLoadComplete();
        m_soundAsset = eventAsset->GetSound();

        m_startFunc = DefaultTask(eventAsset->GetSound());
        eventAsset = {};
    }

    void SaEvent::Update(float)
    {
    }

    void SaEvent::Start(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Start(objectId: %llu)", objectId);
        m_startFunc ? m_startFunc(objectId) : void();
    }

    void SaEvent::Stop(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Stop(objectId: %llu)", objectId);
        m_stopFunc ? m_stopFunc(objectId) : void();
    };
}  // namespace SteamAudio
