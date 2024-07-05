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
    struct DefaultStartFunc
    {
        DefaultStartFunc()
            : m_sound{}
        {
        }

        DefaultStartFunc(AZ::Data::Asset<SaSoundAsset> const& asset)
            : m_asset{ asset }
        {
            m_asset ? m_asset->CopySoundInto(&m_sound) : void();
        }

        DefaultStartFunc(DefaultStartFunc const& other)
            : m_asset{ other.m_asset }
        {
            other.m_asset ? other.m_asset->CopySoundInto(&m_sound) : void();
        }

        DefaultStartFunc(DefaultStartFunc&& other)
            : m_asset(AZStd::move(other.m_asset))
        {
            other.m_asset = {};
            ma_sound_uninit(&other.m_sound);

            m_asset ? m_asset->CopySoundInto(&m_sound) : void();
        }

        ~DefaultStartFunc() = default;

        auto operator=(DefaultStartFunc const& other) -> DefaultStartFunc&
        {
            m_asset = other.m_asset;
            ma_sound_uninit(&m_sound);
            other.m_asset->CopySoundInto(&m_sound);

            return *this;
        }
        auto operator=(DefaultStartFunc&& other) -> DefaultStartFunc&
        {
            ma_sound_uninit(&m_sound);
            ma_sound_uninit(&other.m_sound);

            m_asset = AZStd::move(other.m_asset);
            other.m_asset = {};

            m_asset ? m_asset->CopySoundInto(&m_sound) : void();

            return *this;
        }

        void operator()()
        {
        }

        AZ::Data::Asset<SaSoundAsset> m_asset{};
        ma_sound m_sound{};
    };

    SaEvent::SaEvent() = default;

    SaEvent::SaEvent(AZ::Data::AssetId eventAssetId)
        : m_startFunc()
    {
        auto eventAsset{ AZ::Data::AssetManager::Instance().GetAsset<SaEventAsset>(
            eventAssetId, AZ::Data::AssetLoadBehavior::PreLoad) };
        eventAsset.BlockUntilLoadComplete();

        m_startFunc = DefaultStartFunc(eventAsset->GetSound());
        eventAsset = {};
    }

    void SaEvent::Update(float)
    {
    }

    void SaEvent::Start(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Start(objectId: %llu)", objectId);
        m_startFunc();
    }

    void SaEvent::Stop(SaGameObjectId objectId)
    {
        AZLOG(LOG_SaEvent, "SaEvent::Stop(objectId: %llu)", objectId);
        m_stopFunc();
    };
}  // namespace SteamAudio
