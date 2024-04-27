#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "IAudioInterfacesCommonData.h"

namespace SteamAudio
{
    class AudioEventAsset : public AZ::Data::AssetData
    {
    public:
        AZ_DISABLE_COPY_MOVE(AudioEventAsset);
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(AudioEventAsset);

        static constexpr auto SourceExtension{ "steamaudioeventsrc" };
        static constexpr auto ProductExtension{ "steamaudioevent" };
        static constexpr auto SourceExtensionPattern{ ".steamaudioeventsrc" };
        static constexpr auto ProductExtensionPattern{ ".steamaudioevent" };
        static constexpr auto AssetGroup = "Sound";
        static constexpr auto AssetSubId = 1u;

        static void Reflect(AZ::ReflectContext* context);

        AudioEventAsset();
        ~AudioEventAsset() override;

        [[nodiscard]] auto GetEventId() const -> Audio::TAudioEventID
        {
            return m_id;
        }
        
        void SetEventId(Audio::TAudioEventID id)
        {
            m_id = id;
        }

        [[nodiscard]] auto GetEventName() const -> AZStd::string const&
        {
            return m_eventName;
        }

    private:
        Audio::TAudioEventID m_id{};
        AZStd::string m_eventName{};
    };
} // namespace SteamAudio