#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/IO/FileIO.h"
#include "AzCore/IO/Path/Path.h"
#include "AzCore/Module/Environment.h"
#include "AzCore/std/containers/vector.h"
#include "Engine/ISoundEngine.h"

extern "C" {
struct ma_engine;
struct ma_resource_manager;
}

namespace SteamAudio::Util
{

    inline auto GetAssetPath(AZ::Data::AssetId const& id) -> AZ::IO::Path
    {
        return [&id]() -> decltype(GetAssetPath(id))
        {
            auto result{ decltype(GetAssetPath(id)){} };
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                result, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetPathById, id);

            return result;
        }();
    }

    class Defer
    {
        using DeferFunc = AZStd::function<void(void)>;

    public:
        AZ_DISABLE_COPY_MOVE(Defer);
        Defer(DeferFunc&& func)
            : m_func(func)
        {
        }

        Defer() = delete;
        ~Defer()
        {
            m_func();
        }

    private:
        DeferFunc m_func;
    };

    AZ_TYPE_INFO_SPECIALIZE_WITH_NAME(
        AZStd::byte, "30D9E644-F617-4F06-8B33-F0E04470E7B9", "AZStdByte");

    static inline auto LoadFileIntoBuffer(AZStd::string const& filename)
        -> AZStd::vector<AZStd::byte>
    {
        auto* const fs{ AZ::IO::FileIOBase::GetInstance() };
        if (!fs)
        {
            AZ_Error("Testing", fs != nullptr, "FileIO is null!");
            return {};
        }

        AZ::IO::HandleType fileHandle{};

        AZ::IO::Result const openInputFileResult = fs->Open(
            filename.c_str(),
            AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeBinary,
            fileHandle);

        if (openInputFileResult != AZ::IO::ResultCode::Success)
        {
            AZ_Error("Testing", false, "Unable to open '%s'", filename.c_str());
            fs->Close(fileHandle);
            return {};
        }

        AZ::u64 filesize{};
        fs->Size(fileHandle, filesize);
        AZStd::vector<AZStd::byte> buffer(filesize);
        fs->Read(fileHandle, buffer.data(), filesize, true);

        AZ_Info(
            "LoadAudioBuffer",
            "Read '%s' | Size: %llu | Bytes Read: %llu",
            filename.c_str(),
            buffer.size(),
            filesize);

        return buffer;
    }

    static inline auto GetMaEngine() -> ma_engine*
    {
        auto const maEngine{ AZ::Environment::FindVariable<ma_engine*>(s_lowLevelEngineEnvName) };

        if (!maEngine.IsConstructed())
        {
            AZ_Error(AZ_FUNCTION_SIGNATURE, false, "ma_engine is not available!");
            return nullptr;
        }

        AZ_Error("GetMaEngine", maEngine.Get(), "Found nullptr for ma_engine!");

        return maEngine.Get();
    }

}  // namespace SteamAudio::Util
