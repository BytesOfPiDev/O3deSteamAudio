#pragma once

#include "AzCore/Math/Crc.h"
#include "AzCore/Math/Uuid.h"
#include "AzCore/Name/Name.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "IAudioInterfacesCommonData.h"
#include "IAudioSystem.h"

namespace SteamAudio
{
    class SaIdBase
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SaIdBase);

        SaIdBase() = default;
        explicit SaIdBase(AZStd::string_view name)
            : SaIdBase{ AZ::Name{ name } } {};

        explicit SaIdBase(AZ::Name name)
            : m_name{ AZStd::move(name) }
            , m_value{ name.GetStringView() } {};

        ~SaIdBase() = default;

        explicit SaIdBase(AZ::Uuid const& value)
            : m_name{ value.ToFixedString().c_str() }
            , m_value{ value }
        {
        }

        auto operator==(SaIdBase const& other) const -> bool
        {
            return m_value == other.m_value;
        }

        auto operator!=(SaIdBase const& other) const -> bool
        {
            return !((*this) == other);
        }

        operator AZ::u64() const
        {
            return m_name.GetHash();
        }

        [[nodiscard]] auto GetName() const -> AZStd::string_view
        {
            if (m_name.IsEmpty())
            {
                return {};
            }

            return m_name.GetStringView();
        }

        [[nodiscard]] auto IsValid() const -> bool
        {
            return m_value.IsNull();
        }

    private:
        AZ::Name m_name{};
        AZ::Uuid m_value{};
    };

    using SaId = SaIdBase;
    using SaInstanceId = AZ::u32;
    static constexpr auto InvalidInstanceId{ 0 };

    using SaGameObjectId = AZ::u64;
    static constexpr auto InvalidSaGameObjectId{ INVALID_AUDIO_OBJECT_ID };

    struct SaEventId
    {
        AZ_TYPE_INFO_WITH_NAME(SaEventId, "SaEventId", "9D910540-BC73-424F-8C05-3236DDAC4E96");

        static void Reflect(AZ::ReflectContext* context)
        {
            if (auto* const serialize{ azrtti_cast<AZ::SerializeContext*>(context) })
            {
                serialize->Class<SaEventId>()->Version(0)->Field("Value", &SaEventId::m_value);
            }
        }

        SaEventId() = default;
        constexpr explicit SaEventId(AZ::Crc32 crc32)
            : m_value{ crc32 } {};
        constexpr explicit SaEventId(Audio::TAudioTriggerImplID eventId)
            : m_value{ eventId } {};

        SaEventId(AZStd::string_view eventName)
            : m_value{ Audio::AudioStringToID<Audio::TAudioTriggerImplID>(eventName.data()) }
        {
        }

        [[nodiscard]] auto GetValue() const -> Audio::TAudioTriggerImplID
        {
            return m_value;
        }

        constexpr explicit operator size_t() const
        {
            return m_value;
        }

        constexpr explicit operator Audio::TAudioTriggerImplID() const
        {
            return m_value;
        }

        constexpr auto operator==(SaEventId const& other) const -> bool
        {
            return m_value == other.m_value;
        }
        constexpr auto operator!=(SaEventId const& other) const -> bool
        {
            return !operator==(other);
        }

        [[nodiscard]] constexpr auto IsValid() const
        {
            return m_value != INVALID_AUDIO_TRIGGER_IMPL_ID;
        }

        Audio::TAudioTriggerImplID m_value;
    };

    static_assert(AZStd::is_pod<SaEventId>(), "Must be POD");

    static constexpr auto InvalidEventId{ SaEventId{ INVALID_AUDIO_TRIGGER_IMPL_ID } };

    struct SaEventInstanceId
    {
        AZ_TYPE_INFO_WITH_NAME(
            SaEventInstanceId, "SaEventInstanceId", "22DE65F5-E40F-4951-849B-A4DE70134BA8");

        static void Reflect(AZ::ReflectContext* context)
        {
            if (auto* const serialize{ azrtti_cast<AZ::SerializeContext*>(context) })
            {
                serialize->Class<SaEventInstanceId>()->Version(0)->Field(
                    "Value", &SaEventInstanceId::m_value);
            }
        }

        SaEventInstanceId() = default;
        constexpr explicit SaEventInstanceId(AZ::Crc32 crc32)
            : m_value{ crc32 } {};

        constexpr explicit SaEventInstanceId(Audio::TAudioTriggerInstanceID eventInstanceId)
            : m_value{ eventInstanceId } {};

        constexpr operator Audio::TAudioTriggerInstanceID() const
        {
            return m_value;
        }

        Audio::TAudioTriggerInstanceID m_value;
    };

    static constexpr auto InvalidEventInstanceId{ SaEventInstanceId{
        INVALID_AUDIO_TRIGGER_INSTANCE_ID } };

    namespace Events
    {
        static constexpr auto MuteAllEventName{ "mute_all" };
        static constexpr auto UnmuteAllEventName{ "unmute_all" };
        static constexpr auto GetFocusEventName{ "get_focus" };
        static constexpr auto LoseFocusEventName{ "lose_focus_event" };
        static constexpr auto DoNothingEventName{ "do_nothing" };
        // Used to verify basic functionality of the event system
        static constexpr auto HelloWorldEventName{ "hello_world" };
    }  // namespace Events
}  // namespace SteamAudio
