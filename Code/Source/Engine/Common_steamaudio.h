#pragma once

#include "AzCore/IO/Path/Path.h"
#include "AzCore/Outcome/Outcome.h"

namespace SteamAudio::XmlTags
{
    // Xml Element Names
    static constexpr auto TriggerTag = "SaAudioTrigger";
    static constexpr auto RtpcTag = "SteamAudioRtpc";
    static constexpr auto SwitchTag = "SteamAudioSwitch";
    static constexpr auto GameStateTag = "SteamAudioState";
    static constexpr auto RtpcSwitchTag = "SteamAudioRtpc";
    static constexpr auto SteamAudioFileTag = "SteamAudioFile";
    static constexpr auto AuxBusTag = "SteamAudioAuxBus";
    static constexpr auto ValueTag = "SteamAudioValue";
    static constexpr auto NameAttribute = "steamaudio_name";
    static constexpr auto ValueAttribute = "steamaudio_value";
    static constexpr auto SoundFileAttribute = "steamaudio_soundfile";
    static constexpr auto MultiplierAttribute = "atl_mult";
    static constexpr auto ShiftAttribute = "atl_shift";
    static constexpr auto LocalizedAttribute = "steamaudio_localized";

}  // namespace SteamAudio::XmlTags

namespace AudioStrings
{
    // Project Folders
    static constexpr auto GameParametersFolder = "GameParameters";
    static constexpr auto EventsFolder = "events";
    static constexpr auto BanksFolder = "banks";

    // Xml Tags
    static constexpr auto SoundBankTag = "Bank";
    static constexpr auto EventTag = "Event";
    static constexpr auto TaskTag = "Task";
    static constexpr auto SoundTag = "Sound";
    static constexpr auto NameAttribute = "Name";
    static constexpr auto TypeAttribute = "Type";
    static constexpr auto SoundAttribute = "Sound";
    static constexpr auto SoundFile = "SoundFile";
    static constexpr auto ValueAttribute = "Value";
    static constexpr auto AuxBusTag = "AuxBus";
    static constexpr auto SwitchGroupTag = "SwitchGroup";
    static constexpr auto StateGroupTag = "StateGroup";
    static constexpr auto ChildrenListTag = "ChildrenList";

}  // namespace AudioStrings

namespace JsonKeys
{
    static constexpr AZ::IO::PathView SoundsKey_O = "/SteamAudioDocument/Sounds";
    static constexpr AZ::IO::PathView EventsKey_O{ "/SteamAudioDocument/Events" };
    static constexpr AZ::IO::PathView EventDocNameKey_S{ "/EventDocument/Name" };
}  // namespace JsonKeys

namespace SteamAudio
{
    using EngineNullOutcome = AZ::Outcome<void, AZStd::string>;

    static constexpr auto DocumentJsonSchema =
        R"({
        "$schema": "http://json-schema.org/draft-07/schema#",
        "type": "object",
        "properties": {
          "SteamAudioDocument": {
            "type": "object",
            "properties": {
              "Sounds": {
                "type": "object",
                "additionalProperties": {
                  "type": "object"
                }
              },
              "Events": {
                "type": "object",
                "additionalProperties": {
                  "type": "object",
                  "properties": {
                    "Tasks": {
                      "type": "array",
                      "items": {
                        "type": "object",
                        "properties": {
                          "Play": {
                            "type": "object",
                            "properties": {
                              "Resource": {
                                "type": "string"
                              }
                            },
                            "required": [
                              "Resource"
                            ],
                            "additionalProperties": false
                          },
                          "Stop": {
                            "type": "object",
                            "properties": {
                              "Resource": {
                                "type": "string"
                              }
                            },
                            "required": [
                              "Resource"
                            ],
                            "additionalProperties": false
                          }
                        },
                        "additionalProperties": false
                      }
                    }
                  },
                  "required": [
                    "Tasks"
                  ],
                  "additionalProperties": false
                }
              }
            },
            "additionalProperties": false
          }
        },
        "required": [
          "SteamAudioDocument"
        ],
        "additionalProperties": false
      })";
}  // namespace SteamAudio
