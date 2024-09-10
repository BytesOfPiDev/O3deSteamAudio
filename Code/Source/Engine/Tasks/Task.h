#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/Preprocessor/EnumReflectUtils.h"
#include "AzCore/std/any.h"

namespace SteamAudio
{
    AZ_ENUM_CLASS(TaskType, Start, Stop);
    AZ_ENUM_DEFINE_REFLECT_UTILITIES(TaskType);
    AZ_TYPE_INFO_SPECIALIZE_WITH_NAME_DECL(TaskType);

    struct ITaskInstance
    {
        AZ_RTTI_WITH_NAME(ITaskInstance, "ITaskInstance", "5CD6DC62-7EBD-4FE3-9863-7E88C0C96BB6");

        virtual ~ITaskInstance() = default;

        virtual void StartTask() = 0;
        virtual void StopTask() = 0;
    };

    class TaskDefinition
    {
    public:
        AZ_DEFAULT_COPY_MOVE(TaskDefinition);
        AZ_TYPE_INFO_WITH_NAME_DECL(TaskDefinition);

        static void Reflect(AZ::ReflectContext* context);

        TaskDefinition();
        TaskDefinition(AZStd::any config)
            : m_config{ AZStd::move(config) } {};

        ~TaskDefinition() = default;

        [[nodiscard]] auto GetConfig() const -> AZStd::any const&
        {
            return m_config;
        }

    private:
        //! The configuration file that describes to setup the task associated with our asset
        AZStd::any m_config;
        [[maybe_unused]] TaskType m_taskType{};
    };
}  // namespace SteamAudio
