#include "Tools/SteamAudioLoader.h"
#include <AzTest/AzTest.h>

#include "Tools/Mocks/MockAudioSystemEditor.h"

TEST(SteamAudioLoaderTests, Placeholder)
{
    SteamAudio::SteamAudioLoader loader{};
    MockAudioSystemEditor editor{};

    loader.Load(&editor);
}
