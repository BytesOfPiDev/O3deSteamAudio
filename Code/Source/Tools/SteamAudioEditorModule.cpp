
#include <SteamAudio/SteamAudioTypeIds.h>
#include <SteamAudioModuleInterface.h>
#include "SteamAudioEditorSystemComponent.h"

namespace SteamAudio
{
    class SteamAudioEditorModule
        : public SteamAudioModuleInterface
    {
    public:
        AZ_RTTI(SteamAudioEditorModule, SteamAudioEditorModuleTypeId, SteamAudioModuleInterface);
        AZ_CLASS_ALLOCATOR(SteamAudioEditorModule, AZ::SystemAllocator);

        SteamAudioEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                SteamAudioEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<SteamAudioEditorSystemComponent>(),
            };
        }
    };
}// namespace SteamAudio

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), SteamAudio::SteamAudioEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_SteamAudio_Editor, SteamAudio::SteamAudioEditorModule)
#endif
