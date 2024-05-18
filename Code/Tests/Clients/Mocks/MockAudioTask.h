#pragma once

#include <AzTest/AzTest.h>

#include "Engine/AudioEvent.h"

SteamAudio::PerformFunc MockPerformFunc = [](SteamAudio::AudioObject*)
{
};
class MockAudioTaskBase : SteamAudio::CustomTask
{
public:
    explicit MockAudioTaskBase(SteamAudio::PerformFunc func)
        : SteamAudio::CustomTask{ AZStd::move(func) } {};

    MOCK_METHOD1(CallFunc, void(SteamAudio::AudioObject*));
};

class MockUserTask : public MockAudioTaskBase
{
public:
    explicit MockUserTask(SteamAudio::PerformFunc func)
        : MockAudioTaskBase{ AZStd::move(func) } {};
    MOCK_METHOD1(Perform, void(SteamAudio::AudioObject*));
};
