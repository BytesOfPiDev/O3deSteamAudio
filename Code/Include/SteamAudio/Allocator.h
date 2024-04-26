#pragma once

#include "AzCore/Memory/ChildAllocatorSchema.h"
#include "AzCore/Memory/SystemAllocator.h"

namespace SteamAudio
{
    AZ_CHILD_ALLOCATOR_WITH_NAME(
        SteamAudioAllocator,
        "SteamAudioAllocator",
        "{AE3819D6-DCAE-452E-9E58-6ED7475ACDDA}",
        AZ::SystemAllocator)

    using SteamAudioStdAllocator = SteamAudioAllocator_for_std_t;
} // namespace SteamAudio