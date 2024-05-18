#pragma once

#include "AzCore/Math/Uuid.h"
#include "AzCore/Name/Name.h"
#include "IAudioInterfacesCommonData.h"

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

        [[nodiscard]] auto GetName() const -> AZStd::string
        {
            if (m_name.IsEmpty())
            {
                return {};
            }

            return AZStd::string{ m_name.GetStringView() };
        }

    private:
        AZ::Name m_name{};
        AZ::Uuid m_value{};
    };

    using SaId = SaIdBase;
    using SaInstanceId = AZ::u32;
    static constexpr auto InvalidInstanceId{ 0 };

    using SaGameObjectId = AZ::u64;  // Mimics AZ::EntityId

    using SaEventId = Audio::TAudioTriggerImplID;
    using SaEventInstanceId = Audio::TAudioTriggerInstanceID;

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
