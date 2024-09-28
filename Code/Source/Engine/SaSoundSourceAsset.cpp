#include "Engine/SaSoundSourceAsset.h"

#include <AzCore/Asset/AssetSerializer.h>

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace SteamAudio
{
    void SaSoundSourceData::Reflect(AZ::ReflectContext* context)
    {
        if (auto* const serialize{ azrtti_cast<AZ::SerializeContext*>(context) })
        {
            serialize->Class<SaSoundSourceData, AZ::Data::AssetData>()
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Version(SaSoundSourceData::CurrentVersion)
                ->Field("Sound", &SaSoundSourceData::m_asset)
                ->Field("Name", &SaSoundSourceData::m_name)
                ->Field("NameAsString", &SaSoundSourceData::m_nameStr)
                ->Field("SoundSettings", &SaSoundSourceData::m_soundSettings);

            if (AZ::EditContext* const edit{ serialize->GetEditContext() })
            {
                edit->Class<SaSoundSourceData>("Sound Source Data", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "SteamAudio")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SaSoundSourceData::m_asset, "Sound", "")
                    ->Attribute(
                        AZ::Edit::Attributes::ChangeNotify, &SaSoundSourceData::OnSoundAssetChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SaSoundSourceData::m_nameStr, "Name", "")
                    ->Attribute(
                        AZ::Edit::Attributes::ChangeNotify, &SaSoundSourceData::OnNameChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &SaSoundSourceData::m_soundSettings,
                        "Basic Sound Settings",
                        "");
            }

            serialize->RegisterGenericType<AZ::Data::Asset<SaSoundSourceData>>();
        }
    }
    void SaSoundSourceData::OnSoundAssetChanged()
    {
    }

    void SaSoundSourceData::OnNameChanged()
    {
        m_name = AZ::Name{ m_nameStr };
    };
}  // namespace SteamAudio
