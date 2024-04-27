#include "Clients/AudioEventAsset.h"

#include <AzCore/RTTI/RTTI.h>

#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

namespace SteamAudio
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        AudioEventAsset, "AudioEventAsset", "{0B93821B-098D-4E89-AF08-8D85C6A338EF}")
    AZ_RTTI_NO_TYPE_INFO_IMPL(AudioEventAsset, AZ::Data::AssetData);

    void AudioEventAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<AudioEventAsset>()->Version(0);

            if (auto* editContext = serialize->GetEditContext())
            {
                editContext->Class<AudioEventAsset>("AudioEventAsset", "Audio event asset")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }
    }

    AudioEventAsset::AudioEventAsset()
    {
    }

    AudioEventAsset::~AudioEventAsset()
    {
    }
} // namespace SteamAudio