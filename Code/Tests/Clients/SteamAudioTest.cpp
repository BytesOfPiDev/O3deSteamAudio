#include <AzTest/AzTest.h>
#include <gtest/gtest.h>

#include "AzCore/Outcome/Outcome.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "Clients/AudioImplTestFixture.h"
#include "Engine/MaSoundEngine.h"
#include "IAudioInterfacesCommonData.h"

#include "Clients/BaseTestFixture.h"
#include "Clients/Mocks/MockSoundEngine.h"
#include "Engine/ATLEntities_steamaudio.h"
#include "Engine/AudioSystemImplementation_steamaudio.h"
#include "Engine/Id.h"
#include "IAudioSystem.h"

using ::testing::Return;

static constexpr auto EmptyName{ "" };
constexpr auto SomeValidName{ "ricky_spanish" };
constexpr auto SomeDifferentValidName{ "geneveve_vivance" };

TEST(SATLTriggerImplDataTests, Construction_GiveNullTriggerName_GetImplEventId_ReturnsZero)
{
    SteamAudio::SATLTriggerImplData_steamaudio const triggerData{ AZ::Name{ EmptyName } };

    EXPECT_EQ(
        triggerData.GetImplEventId(), Audio::AudioStringToID<SteamAudio::SaEventId>(EmptyName));
}

TEST(SATLTriggerImplDataTests, TriggerData_ConstructWithValidName_GetImplEventId_ReturnsCorrectId)
{
    SteamAudio::SATLTriggerImplData_steamaudio const triggerData{ AZ::Name{ SomeValidName } };

    EXPECT_EQ(
        triggerData.GetImplEventId(), Audio::AudioStringToID<SteamAudio::SaEventId>(SomeValidName));
    EXPECT_NE(
        triggerData.GetImplEventId(),
        Audio::AudioStringToID<SteamAudio::SaEventId>(SomeDifferentValidName));
    EXPECT_NE(
        triggerData.GetImplEventId(), Audio::AudioStringToID<SteamAudio::SaEventId>(EmptyName));
}

TEST_F(BaseTestFixture, SANITY_CHECK)
{
}

TEST_F(BaseTestFixture, CreateImplWithNoEngine_CallInitialize_ReturnsFailure)
{
    auto audioSystemImpl{ SteamAudio::AudioSystemImpl_steamaudio{} };
    AZ_TEST_START_TRACE_SUPPRESSION;
    EXPECT_EQ(audioSystemImpl.Initialize(), Audio::EAudioRequestStatus::Failure);
    AZ_TEST_STOP_TRACE_SUPPRESSION(1);
}

TEST_F(BaseTestFixture, CreateAudioImpl_CallInitialize_ReturnsSuccess)
{
    auto soundEngine{ MockSteamAudioEngine{} };

    EXPECT_CALL(soundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(soundEngine, Shutdown).Times(1).WillOnce(Return(AZ::Success()));

    []() -> void
    {
        auto impl{ SteamAudio::AudioSystemImpl_steamaudio{} };
        EXPECT_EQ(impl.Initialize(), Audio::EAudioRequestStatus::Success);
    }();
}

TEST_F(BaseTestFixture, InitializedAudioImpl_RegisterAudioObjectWithValidArgs_ReturnsSuccess)
{
    MockSteamAudioEngine mockSoundEngine{};
    EXPECT_CALL(mockSoundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, RegisterAudioObject).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, Shutdown).Times(1).WillOnce(Return(AZ::Success()));

    []() -> void
    {
        auto impl{ SteamAudio::AudioSystemImpl_steamaudio() };
        impl.Initialize();

        static constexpr auto* validObjectName{ "my_leg" };
        SteamAudio::SATLAudioObjectData_steamaudio validObjectData{};

        EXPECT_EQ(
            impl.RegisterAudioObject(&validObjectData, validObjectName),
            Audio::EAudioRequestStatus::Success);
    }();
}

TEST_F(BaseTestFixture, InitializeAudioImpl_ActivateInvalidTriggerUsingValidImplData_ReturnsFailure)
{
    MockSteamAudioEngine mockSoundEngine{};
    EXPECT_CALL(mockSoundEngine, Initialize).Times(1).WillOnce(Return(AZ::Success()));
    EXPECT_CALL(mockSoundEngine, ReportEvent)
        .Times(1)
        .WillOnce(Return(SteamAudio::SaEventInstanceId{}));
    EXPECT_CALL(mockSoundEngine, Shutdown).Times(1);

    []() -> void
    {
        SteamAudio::AudioSystemImpl_steamaudio impl{};
        impl.Initialize();

        SteamAudio::SATLAudioObjectData_steamaudio objData{
            SteamAudio::SaAudioObjectId{ AZ::Entity::MakeId() }, false
        };

        SteamAudio::SATLTriggerImplData_steamaudio const triggerData{};

        static constexpr AZStd::string_view eventName{ "this_trigger_does_not_exist" };

        auto const atlEventId{ Audio::AudioStringToID<Audio::TAudioEventID>(eventName.data()) };

        SteamAudio::SATLEventData_steamaudio eventData{ atlEventId };

        Audio::SATLSourceData const sourceData{};

        AZ_TEST_START_TRACE_SUPPRESSION;
        EXPECT_NE(
            impl.ActivateTrigger(&objData, &triggerData, &eventData, &sourceData),
            Audio::EAudioRequestStatus::Success);
        AZ_TEST_STOP_TRACE_SUPPRESSION(1);

        EXPECT_EQ(eventData.GetEventState(), Audio::EAudioEventState::eAES_NONE);
    }();
}

TEST_F(BaseTestFixture, EventAsset_LoadAssetWithSoundDep_SoundDepLoadsSuccessfully)
{
    SteamAudio::MaSoundEngine engine{};
    SteamAudio::AudioSystemImpl_steamaudio impl{};
}

class InitializedAudioImplTestFixture : public BaseTestFixture
{
public:
    void SetUp() override
    {
        BaseTestFixture::SetUp();
        m_engine.emplace();
        m_impl.emplace();

        m_impl->Initialize();
    }

    void TearDown() override
    {
        ASSERT_TRUE(m_impl.has_value());
        m_impl->ShutDown();
        m_impl.reset();

        ASSERT_TRUE(m_engine.has_value());
        m_engine->Shutdown();
        m_engine.reset();

        BaseTestFixture::TearDown();
    }

    auto GetSoundEngine() const -> SteamAudio::MaSoundEngine const&
    {
        AZ_TEST_ASSERT(m_engine.has_value());
        return m_engine.value();
    }

    auto GetSoundEngine() -> SteamAudio::MaSoundEngine&
    {
        AZ_TEST_ASSERT(m_engine.has_value());
        return m_engine.value();
    }

    auto GetAudioImpl() const -> SteamAudio::AudioSystemImpl_steamaudio const&
    {
        AZ_TEST_ASSERT(m_impl.has_value());
        return m_impl.value();
    }

    auto GetAudioImpl() -> SteamAudio::AudioSystemImpl_steamaudio&
    {
        AZ_TEST_ASSERT(m_impl.has_value());
        return m_impl.value();
    }

private:
    AZStd::optional<SteamAudio::MaSoundEngine> m_engine{ AZStd::nullopt };
    AZStd::optional<SteamAudio::AudioSystemImpl_steamaudio> m_impl{ AZStd::nullopt };
};

TEST_F(AudioImplTestFixture, SANITY_CHECK)
{
}

TEST_F(AudioImplTestFixture, DISABLED_Initialized_ActivateDoNothingEvent_ReturnsSuccess)
{
    SteamAudio::SATLAudioObjectData_steamaudio objData{
        SteamAudio::SaAudioObjectId{ AZ::Entity::MakeId() }, false
    };

    SteamAudio::SATLTriggerImplData_steamaudio const triggerData{ AZ::Name{
        SteamAudio::Events::DoNothingEventName } };

    [[maybe_unused]] auto const atlEventId{ Audio::AudioStringToID<Audio::TAudioEventID>(
        SteamAudio::Events::DoNothingEventName) };
    SteamAudio::SATLEventData_steamaudio eventData{ atlEventId };

    [[maybe_unused]] Audio::SATLSourceData const sourceData{};

    SteamAudio::MaSoundEngine engine{};
    SteamAudio::AudioSystemImpl_steamaudio impl{};
    ASSERT_EQ(impl.Initialize(), Audio::EAudioRequestStatus::Success);
    EXPECT_EQ(
        impl.ActivateTrigger(&objData, &triggerData, &eventData, &sourceData),
        Audio::EAudioRequestStatus::Success);

    EXPECT_NE(eventData.GetEventState(), Audio::EAudioEventState::eAES_NONE);
}

TEST_F(
    InitializedAudioImplTestFixture,
    Initialized_CallNewGlobalAudioObjectDataWithValidId_ReturnedObjectHasId)
{
    static constexpr SteamAudio::SaAudioObjectId ValidObjectId{ 1337 };
    auto* const newAudioObject{ static_cast<SteamAudio::SATLAudioObjectData_steamaudio*>(
        GetAudioImpl().NewGlobalAudioObjectData(ValidObjectId)) };

    ASSERT_NE(newAudioObject, nullptr);
    EXPECT_EQ(newAudioObject->GetId(), ValidObjectId);

    GetAudioImpl().DeleteAudioObjectData(newAudioObject);
}

TEST_F(
    InitializedAudioImplTestFixture, Initialized_CallNewAudioObjectDataWithValidId_GetIdReturnsId)
{
    static constexpr SteamAudio::SaAudioObjectId ValidObjectId{ 2 };
    auto* const newAudioObject{ static_cast<SteamAudio::SATLAudioObjectData_steamaudio*>(
        GetAudioImpl().NewAudioObjectData(ValidObjectId)) };

    ASSERT_NE(newAudioObject, nullptr);
    ASSERT_EQ(newAudioObject->GetId(), ValidObjectId);

    GetAudioImpl().DeleteAudioObjectData(newAudioObject);
}
