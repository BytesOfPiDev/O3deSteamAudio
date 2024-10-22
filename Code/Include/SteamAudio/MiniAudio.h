#pragma once

#include "AzCore/RTTI/TypeInfoSimple.h"
#include "miniaudio.h"

AZ_TYPE_INFO_SPECIALIZE_WITH_NAME(ma_engine, "{20216DDD-6B5E-489D-80BC-27DB9866D53B}", "ma_engine");
AZ_TYPE_INFO_SPECIALIZE_WITH_NAME(ma_sound, "{272C63E2-F02F-46A2-B351-89E99928D95F}", "ma_sound");
AZ_TYPE_INFO_SPECIALIZE_WITH_NAME(ma_device, "{6535500A-47ED-4ADD-8A99-D073AF424363}", "ma_device");
AZ_TYPE_INFO_SPECIALIZE_WITH_NAME(
    ma_resource_manager, "{43D96323-4623-48E0-BE13-7D9B0BC11B48}", "ma_resource_manager");
