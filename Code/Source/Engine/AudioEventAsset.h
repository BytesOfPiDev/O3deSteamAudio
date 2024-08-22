#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/std/smart_ptr/shared_ptr.h"
#include "AzFramework/Asset/GenericAssetHandler.h"
#include "Engine/AudioEvent.h"
#include "Engine/Id.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"

extern "C" {
struct ma_sound;
}

namespace SteamAudio
{
    using AudioEventName = AZStd::string;

    class SaEventAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(SaEventAsset);
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(SaEventAsset);

        friend class AudioEventAssetHandler;
        friend class MiniAudioEngine;

        static constexpr auto ProductExtension{ "saevent" };
        static constexpr auto ProductExtensionWildcard{ "*.saevent" };
        static constexpr auto ProductExtensionRegex{
            R"((.*sounds\/steamaudio\/events\/).*\.saevent)"
        };

        static constexpr auto SourceExtension{ "xml" };
        static constexpr auto SourceExtensionWildcard{ "*.xml" };
        static constexpr auto SourceExtensionRegex{ R"((.*sounds\/steamaudio\/events\/).*\.xml)" };

        static constexpr auto AssetGroup = "Sound";
        static constexpr auto AssetSubId = 0u;

        using SetupFunc = AZStd::function<AZStd::unique_ptr<SaEvent>(AZ::Data::AssetId)>;

        static void Reflect(AZ::ReflectContext* context);

        /*
         * Default constructor.
         *
         * TODO: Make protected after figuring out how to serialize classes with protected
         * constructor.
         */
        SaEventAsset();
        explicit SaEventAsset(AudioEventName eventName);
        SaEventAsset(AudioEventName eventName, SetupFunc setupFunc);
        ~SaEventAsset() override;

        void SetEventName(AudioEventName eventName);

        void SetSound(AZ::Data::Asset<SaSoundAsset> soundAsset);

        [[nodiscard]] auto GetEventName() const -> AudioEventName
        {
            return m_name;
        }

        [[nodiscard]] auto GetEventId() const -> Audio::TAudioEventID
        {
            return m_eventId;
        }

        [[nodiscard]] auto GetSound() const -> AZ::Data::Asset<SaSoundAsset>
        {
            return m_soundAsset;
        }

        [[nodiscard]] auto CreateInstance() const -> AZStd::unique_ptr<SaEvent>;

        void PlayEvent() const
        {
        }

        void ChangeSetupFunc(SetupFunc setupFunc)
        {
            m_setupFunc = AZStd::move(setupFunc);
        }

        void LoadDependencies()
        {
            AZLOG(LOG_SaEventAsset, "Loading sound asset for event %s", m_name.c_str());
            if (!m_soundAsset.GetId().IsValid())
            {
                return;
            }

            m_soundAsset.QueueLoad();
            m_soundAsset.BlockUntilLoadComplete();
        }

    protected:
        void UpdateId();

    private:
        SaEventId m_eventId{};
        AudioEventName m_name{};
        AZ::Data::Asset<SaSoundAsset> m_soundAsset{};
        AZStd::shared_ptr<ma_sound> m_soundInstance{};
        SetupFunc m_setupFunc{};
    };

    using AudioEventAssetDataPtr = AZ::Data::Asset<SaEventAsset>;
    using AudioEventAssets = AZStd::vector<AudioEventAssetDataPtr>;

    using SaEventAssetGenericHandler = AzFramework::GenericAssetHandler<SaEventAsset>;

}  // namespace SteamAudio
