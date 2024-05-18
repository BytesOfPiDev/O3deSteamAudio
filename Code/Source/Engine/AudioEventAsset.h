#pragma once

#include "AzCore/Asset/AssetCommon.h"
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

        static void Reflect(AZ::ReflectContext* context);

        /*
         * Default constructor.
         *
         * TODO: Make protected after figuring out how to serialize classes with protected
         * constructor.
         */
        SaEventAsset();
        explicit SaEventAsset(AudioEventName eventName);
        ~SaEventAsset() override;

        virtual void SetEventName(AudioEventName eventName);

        [[nodiscard]] virtual auto GetEventName() const -> AudioEventName
        {
            return m_name;
        }

        [[nodiscard]] virtual auto GetEventId() const -> Audio::TAudioEventID
        {
            return m_id;
        }

        virtual void SetEventId(AudioEventName eventName)
        {
            SetEventName(AZStd::move(eventName));
        }

        auto CloneEvent() -> SaId
        {
            return {};
        }

        void PlayEvent() const
        {
        }

    private:
        SaEventId m_id{};
        AudioEventName m_name{};
        AZ::Data::Asset<SaSoundAsset> m_sound{};
    };

    using AudioEventAssetDataPtr = AZ::Data::Asset<SaEventAsset>;
    using AudioEventAssets = AZStd::vector<AudioEventAssetDataPtr>;

}  // namespace SteamAudio
