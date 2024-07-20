#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/Name/Name.h"
#include "Engine/Sound.h"

namespace SteamAudio
{
    class SaSoundAsset;

    void CopySound(ma_sound const* fromSound, ma_sound* toSound);
    void CreateSound(AZStd::string_view soundName, ma_sound* sound);
    auto PathToSoundName(AZStd::string_view filePath) -> AZStd::string;

    class SoundResourceManagerRequests
    {
    public:
        AZ_DEFAULT_COPY_MOVE(SoundResourceManagerRequests);

        SoundResourceManagerRequests() = default;
        virtual ~SoundResourceManagerRequests() = default;

        virtual auto RegisterSound(SaSoundAsset* soundData, AZ::Name soundName) -> bool = 0;
        virtual auto UnregisterSound(AZ::Name soundName) -> bool = 0;

        virtual auto CreateSound(AZ::Name soundName) -> AZ::Outcome<Sound, AZStd::string> = 0;
    };

    struct SoundResourceManagerRequestBusTraits : public AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        /**
         * Locking primitive that is used when connecting handlers to the EBus or executing events.
         * By default, all access is assumed to be single threaded and no locking occurs.
         * For multithreaded access, specify a mutex of the following type.
         * - For simple multithreaded cases, use AZStd::mutex.
         * - For multithreaded cases where an event handler sends a new event on the same bus
         *   or connects/disconnects while handling an event on the same bus, use
         * AZStd::recursive_mutex.
         * - For specialized multithreading cases, such as allowing events to execute in parallel
         * with each other but not with connects / disconnects, use custom mutex types along with
         * custom LockGuard policies to control the specific locking requirements for each mutex use
         * case (connection, dispatch, binding, callstack tracking).
         */
        using MutexType = AZStd::mutex;

        /**
         * Specifies whether the EBus supports an event queue.
         * You can use the event queue to execute events at a later time.
         * To execute the queued events, you must call
         * `<BusName>::ExecuteQueuedEvents()`.
         * By default, the event queue is disabled.
         */
        static constexpr bool EnableEventQueue = false;

        /**
         * Specifies whether the bus should accept queued messages by default or not.
         * If set to false, Bus::AllowFunctionQueuing(true) must be called before events are
         * accepted. Used only when #EnableEventQueue is true.
         */
        static constexpr bool EventQueueingActiveByDefault = false;

        /**
         * Specifies whether the EBus supports queueing functions which take reference
         * arguments. This means that the sender is responsible for the lifetime of the
         * arguments (they should be static or class members or otherwise persistently stored).
         * You should only use this if you know that the data being passed as arguments will
         * outlive the dispatch of the queued event.
         */
        static constexpr bool EnableQueuedReferences = false;

        /**
         * Locking primitive that is used when adding and removing
         * events from the queue.
         * Not used for connection or event execution.
         * Used only when #EnableEventQueue is true.
         * If left unspecified, it will use the #MutexType.
         */
        using EventQueueMutexType = AZStd::mutex;

        /**
         * Controls the flow of EBus events.
         * Enables an event to be forwarded, and possibly stopped, before reaching
         * the normal event handlers.
         * Use cases for routing include tracing, debugging, and versioning an %EBus.
         * The default `EBusRouterPolicy` forwards the event to each connected
         * `EBusRouterNode` before sending the event to the normal handlers. Each
         * node can stop the event or let it continue.
         */
        template<class Bus>
        using RouterPolicy = AZ::EBusRouterPolicy<Bus>;

        /*
         * Performs the actual call on an Ebus handler.
         * Enables custom code to be run on a per-callee basis.
         * Use cases include debugging systems and profiling that need to run custom
         * code before or after an event.
         */
        using EventProcessingPolicy = AZ::EBusEventProcessingPolicy;
    };

    using SoundResourceManagerRequestBus =
        AZ::EBus<SoundResourceManagerRequests, SoundResourceManagerRequestBusTraits>;

    static constexpr auto* s_maResMgrEnvName{ "ma_resource_manager" };
    class SoundResourceManager : protected SoundResourceManagerRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundResourceManager);
        AZ_TYPE_INFO_WITH_NAME_DECL(SoundResourceManager);

        SoundResourceManager();
        ~SoundResourceManager() override;

        void Update();

    protected:
        auto RegisterSound(SaSoundAsset* soundData, AZ::Name soundName) -> bool override;
        auto UnregisterSound(AZ::Name soundName) -> bool override;

        auto CreateSound(AZ::Name soundName) -> AZ::Outcome<Sound, AZStd::string> override;

    private:
        AZStd::unordered_set<AZ::Name> m_registeredNames{};
    };

}  // namespace SteamAudio
