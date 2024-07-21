#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "Engine/AudioEvent.h"
#include "Engine/Id.h"
#include "Engine/SoundAsset.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{

    class SaEventAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(SaEventAsset);
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(SaEventAsset);

        friend class AudioEventAssetHandler;
        friend class MiniAudioEngine;

        using AudioEventName = AZStd::string;

        static constexpr auto ProductExtension{ "saevent" };
        static constexpr auto ProductExtensionWildcard{ "*.saevent" };
        static constexpr auto ProductExtensionRegex{
            R"((.*sounds\/steamaudio\/events\/).*\.saevent)"
        };

        static constexpr auto SourceExtension{ "xml" };
        static constexpr auto SourceExtensionWildcard{ "*.xml" };
        static constexpr auto SourceExtensionRegex{ R"((.*sounds\/steamaudio\/events\/).*\.xml)" };

        static constexpr auto AssetGroup = "Sound";
        static constexpr auto AssetSubId = 1u;

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

        virtual void SetEventName(AudioEventName eventName);

        [[nodiscard]] virtual auto GetEventName() const -> AudioEventName
        {
            return m_name;
        }

        [[nodiscard]] virtual auto GetEventId() const -> Audio::TAudioEventID
        {
            return m_eventId;
        }

        [[nodiscard]] virtual auto GetSound() const -> AZ::Data::Asset<SaSoundAsset>
        {
            return m_sound;
        }

        virtual void SetEventId(AudioEventName eventName)
        {
            SetEventName(AZStd::move(eventName));
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
            m_sound.IsReady() ? m_sound.BlockUntilLoadComplete()
                              : AZ::Data::AssetData::AssetStatus();
        }

    protected:
        void UpdateId();

    private:
        SaEventId m_eventId{};
        AudioEventName m_name{};
        AZ::Data::Asset<SaSoundAsset> m_sound{};
        SetupFunc m_setupFunc{};
    };

    using AudioEventAssetDataPtr = AZ::Data::Asset<SaEventAsset>;
    using AudioEventAssets = AZStd::vector<AudioEventAssetDataPtr>;

}  // namespace SteamAudio
