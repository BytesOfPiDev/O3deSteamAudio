
#include <SteamAudio/SteamAudioTypeIds.h>
#include <SteamAudioModuleInterface.h>
#include "SteamAudioSystemComponent.h"

namespace SteamAudio
{
    class SteamAudioModule
        : public SteamAudioModuleInterface
    {
    public:
        AZ_RTTI(SteamAudioModule, SteamAudioModuleTypeId, SteamAudioModuleInterface);
        AZ_CLASS_ALLOCATOR(SteamAudioModule, AZ::SystemAllocator);
    };
}// namespace SteamAudio

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), SteamAudio::SteamAudioModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_SteamAudio, SteamAudio::SteamAudioModule)
#endif
