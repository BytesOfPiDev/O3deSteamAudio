#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzFramework/Asset/GenericAssetHandler.h"
#include "IAudioInterfacesCommonData.h"

#include "Engine/Id.h"
#include "Engine/SaEvent.h"
#include "Engine/Tasks/Task.h"
#include "SteamAudio/SteamAudioTypeIds.h"

extern "C" {
struct ma_sound;
}

namespace SteamAudio
{
    using AudioEventName = AZStd::string;

    class SaEventAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI_WITH_NAME(SaEventAsset, "SaEventAsset", SaEventAssetTypeId, AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(SaEventAsset, Audio::AudioImplAllocator);

        AZ_DISABLE_COPY_MOVE(SaEventAsset);

        friend class MiniAudioEngine;

        static constexpr auto CurrentVersion = 6u;

        static constexpr auto Extension{ "saevent" };
        static constexpr auto ExtensionWildcard{ "*.saevent" };
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
        ~SaEventAsset() override;

        void SetEventName(AudioEventName eventName);

        [[nodiscard]] auto GetEventName() const -> AudioEventName
        {
            return m_name;
        }

        [[nodiscard]] auto GetEventId() const -> SaEventId
        {
            return m_eventId;
        }

        void PlayEvent() const
        {
        }

        void ChangeSetupFunc(SetupFunc setupFunc)
        {
            m_setupFunc = AZStd::move(setupFunc);
        }

        [[nodiscard]] auto GetTasksConfigs() const -> AZStd::vector<TaskDefinition>
        {
            return m_tasks;
        }

    protected:
        void UpdateId();

    private:
        SaEventId m_eventId{};
        AudioEventName m_name{};
        SetupFunc m_setupFunc{};
        AZStd::vector<TaskDefinition> m_tasks{};
    };

    using SaEventDataPtr = AZ::Data::Asset<SaEventAsset>;
    using AudioEventAssets = AZStd::vector<SaEventDataPtr>;

    using SaEventAssetGenericHandler = AzFramework::GenericAssetHandler<SaEventAsset>;

    class RegisteredEventRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(RegisteredEventRequests);

        RegisteredEventRequests() = default;
        virtual ~RegisteredEventRequests() = default;

        [[nodiscard]] virtual auto GetEvent() const -> AZ::Data::Asset<SaEventAsset> = 0;
    };

}  // namespace SteamAudio
