#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzFramework/Asset/GenericAssetHandler.h"
#include "Engine/SoundAsset.h"

namespace SteamAudio
{
    class EditorSaEventAsset : public AZ::Data::AssetData
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(EditorSaEventAsset);
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_CLASS_ALLOCATOR_DECL;

        AZ_DISABLE_COPY_MOVE(EditorSaEventAsset);

        static constexpr auto Extension{ "sesrc" };
        static constexpr auto ExtensionWildcard{ "*.sesrc" };
        static constexpr auto ExtensionRegex{ R"((.*sounds\/steamaudio\/events\/).*\.sesrc)" };
        static constexpr auto AssetSubId{ 2u };

        EditorSaEventAsset() = default;
        ~EditorSaEventAsset() override = default;

        static void Reflect(AZ::ReflectContext* context);

        [[nodiscard]] auto GetEventName() const -> AZStd::string_view
        {
            return m_eventName;
        }

        [[nodiscard]] auto GetSoundToPlay() const -> AZ::Data::Asset<SaSoundAsset>
        {
            return m_soundAsset;
        }

    protected:
        void SetEventName(AZStd::string eventName);

    private:
        AZStd::string m_eventName{};
        AZ::Data::Asset<SaSoundAsset> m_soundAsset{};
    };

    using EditorSaEventAssetHandler = AzFramework::GenericAssetHandler<EditorSaEventAsset>;
}  // namespace SteamAudio
