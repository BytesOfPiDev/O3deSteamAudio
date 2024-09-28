#pragma once

#include "Engine/SaSoundSourceAsset.h"
#include "Engine/SaSoundSourceBus.h"

namespace SteamAudio
{
    class SoundSource : public SaSoundSourceRequestBus::Handler
    {
    public:
        AZ_DISABLE_COPY_MOVE(SoundSource);

        SoundSource() = delete;
        explicit SoundSource(AZ::Data::Asset<SaSoundSourceData> soundSourceAsset);
        ~SoundSource() override;

        [[nodiscard]] auto GetName() const -> AZ::Name
        {
            return m_name;
        }

    protected:
        void Register();
        [[nodiscard]] auto CreateInstance() const -> AZStd::any override;

    private:
        AZ::Name m_name{};
        AZ::Data::Asset<SaSoundSourceData> m_soundSourceAsset;
    };
}  // namespace SteamAudio
