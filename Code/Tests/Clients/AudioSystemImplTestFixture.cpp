#include "Clients/AudioSystemImplTestFixture.h"

#include "AzCore/Component/Component.h"
#include "AzFramework/Application/Application.h"

#include "AzCore/Module/ModuleManagerBus.h"
#include "AzCore/Script/ScriptSystemComponent.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "AzCore/UserSettings/UserSettingsComponent.h"
#include "AzFramework/Application/Application.h"
#include "AzFramework/FileTag/FileTagComponent.h"
#include "AzFramework/Input/System/InputSystemComponent.h"
#include "AzFramework/PaintBrush/PaintBrushSystemComponent.h"
#include "AzFramework/Physics/Material/PhysicsMaterialSystemComponent.h"
#include "AzFramework/Render/GameIntersectorComponent.h"
#include "AzFramework/StreamingInstall/StreamingInstall.h"

#include "AzFramework/Asset/CustomAssetTypeComponent.h"
#include "AzFramework/Spawnable/SpawnableSystemComponent.h"

class EngineTestApplication : public AzFramework::Application
{
protected:
    void StartCommon(AZ::Entity* systemEntity) override
    {
        AzFramework::Application::StartCommon(systemEntity);
    }

    void RegisterCoreComponents() override
    {
        AzFramework::Application::RegisterCoreComponents();
    }

    auto GetRequiredSystemComponents() const -> AZ::ComponentTypeList override
    {
        auto required = AzFramework::Application::GetRequiredSystemComponents();

        AZStd::erase(required, azrtti_typeid<AZ::ScriptSystemComponent>());

        AZStd::erase(required, azrtti_typeid<AzFramework::FileTag::ExcludeFileComponent>());
        AZStd::erase(
            required, azrtti_typeid<AzFramework::RenderGeometry::GameIntersectorComponent>());
        AZStd::erase(required, azrtti_typeid<AzFramework::InputSystemComponent>());
        AZStd::erase(required, azrtti_typeid<AzFramework::PaintBrushSystemComponent>());
        AZStd::erase(
            required,
            azrtti_typeid<AzFramework::StreamingInstall::StreamingInstallSystemComponent>());
        AZStd::erase(required, azrtti_typeid<Physics::MaterialSystemComponent>());

        // NOTE: CustomAssetTypeComponent has XML schema handler.
        AZStd::erase(required, azrtti_typeid<AzFramework::CustomAssetTypeComponent>());

        AZStd::erase(required, azrtti_typeid<AzFramework::SpawnableSystemComponent>());

        return required;
    }
};

void AudioSystemImplTestFixture::SetUp()
{
    m_app = AZStd::make_unique<EngineTestApplication>();

    AZ::ComponentApplication::Descriptor appDesc;
    appDesc.m_modules.push_back({ "AudioSystem" });
    appDesc.m_enableScriptReflection = false;

    AZ::ComponentApplication::StartupParameters startupParams{};
    startupParams.m_loadAssetCatalog = true;

    static_cast<AzFramework::Application*>(m_app.get())->Start(appDesc, startupParams);

    AZ::UserSettingsComponentRequestBus::Broadcast(
        &AZ::UserSettingsComponentRequests::DisableSaveOnFinalize);

    AZ::SerializeContext* serializeContext = nullptr;
    AZ::ComponentApplicationBus::BroadcastResult(
        serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
    AZ_TEST_ASSERT(serializeContext != nullptr);
}

void AudioSystemImplTestFixture::TearDown()
{
    static_cast<AzFramework::Application*>(m_app.get())->Stop();
}
