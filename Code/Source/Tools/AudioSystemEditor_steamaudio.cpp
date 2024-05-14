#include "Tools/AudioSystemEditor_steamaudio.h"

#include "ACETypes.h"
#include "AzCore/StringFunc/StringFunc.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "AzCore/std/string/conversions.h"
#include "IAudioConnection.h"
#include "IAudioSystem.h"
#include "IAudioSystemControl.h"
#include "IAudioSystemEditor.h"

#include "Engine/Common_steamaudio.h"
#include "Engine/Configuration.h"
#include "Tools/AudioSystemControl_steamaudio.h"

namespace SteamAudio
{
    auto TagToType(AZStd::string_view const tag) -> AudioControls::TImplControlType
    {
        if (tag == XmlTags::TriggerTag)
        {
            return SteamAudioControlType::Event;
        }
        else if (tag == XmlTags::RtpcTag)
        {
            return SteamAudioControlType::Rtpc;
        }
        else if (tag == XmlTags::AuxBusTag)
        {
            return SteamAudioControlType::AuxBus;
        }
        else if (tag == XmlTags::SteamAudioFileTag)
        {
            return SteamAudioControlType::SoundBank;
        }
        else if (tag == XmlTags::SwitchTag)
        {
            return SteamAudioControlType::Switch;
        }
        else if (tag == XmlTags::GameStateTag)
        {
            return SteamAudioControlType::GameState;
        }

        return SteamAudioControlType::Invalid;
    }

    auto TypeToTag([[maybe_unused]] AudioControls::TImplControlType const type)
        -> AZStd::string_view
    {
        switch (type)
        {
        case SteamAudioControlType::Event:
            return XmlTags::TriggerTag;
        case SteamAudioControlType::Rtpc:
            return XmlTags::RtpcTag;
        case SteamAudioControlType::Switch:
            return XmlTags::ValueTag;
        case SteamAudioControlType::AuxBus:
            return XmlTags::AuxBusTag;
        case SteamAudioControlType::SoundBank:
            return XmlTags::SteamAudioFileTag;
        case SteamAudioControlType::GameState:
            return XmlTags::ValueTag;
        case SteamAudioControlType::SwitchGroup:
            return XmlTags::SwitchTag;
        case SteamAudioControlType::GameStateGroup:
            return XmlTags::GameStateTag;
        };

        return "";
    }

    AudioSystemEditor_steamaudio::AudioSystemEditor_steamaudio() = default;

    void AudioSystemEditor_steamaudio::Reload()
    {
        AZStd::ranges::for_each(
            m_controls,
            [](auto const& idControlPair)
            {
                TControlPtr control = idControlPair.second;
                if (control)
                {
                    return;
                }

                control->SetPlaceholder(true);
            });

        m_loader.Load(this);

        m_connectionsByID.clear();
        UpdateConnectedStatus();
    }

    auto AudioSystemEditor_steamaudio::CreateControl(
        AudioControls::SControlDef const& controlDefinition) -> AudioControls::IAudioSystemControl*
    {
        AZStd::string fullName = controlDefinition.m_name;

        AudioControls::IAudioSystemControl* parent = controlDefinition.m_parentControl;
        if (parent)
        {
            AZ::StringFunc::Path::Join(
                controlDefinition.m_parentControl->GetName().c_str(), fullName.c_str(), fullName);
        }

        if (!controlDefinition.m_path.empty())
        {
            AZ::StringFunc::Path::Join(
                controlDefinition.m_path.c_str(), fullName.c_str(), fullName);
        }

        AudioControls::CID id = GetID(fullName);

        AudioControls::IAudioSystemControl* control = GetControl(id);
        if (control)
        {
            if (control->IsPlaceholder())
            {
                control->SetPlaceholder(false);
                if (parent && parent->IsPlaceholder())
                {
                    parent->SetPlaceholder(false);
                }
            }
            return control;
        }
        else
        {
            TControlPtr newControl = AZStd::make_shared<AudioSystemControl_steamaudio>(
                controlDefinition.m_name, id, controlDefinition.m_type);

            if (!parent)
            {
                parent = &m_rootControl;
            }

            parent->AddChild(newControl.get());
            newControl->SetParent(parent);
            newControl->SetLocalized(controlDefinition.m_isLocalized);
            m_controls[id] = newControl;

            return newControl.get();
        }
    }

    auto AudioSystemEditor_steamaudio::GetControl(AudioControls::CID id) const
        -> AudioControls::IAudioSystemControl*
    {
        if (id != AudioControls::ACE_INVALID_CID)
        {
            auto it = m_controls.find(id);
            if (it != m_controls.end())
            {
                return it->second.get();
            }
        }

        return nullptr;
    }

    auto AudioSystemEditor_steamaudio::ImplTypeToATLType(AudioControls::TImplControlType type) const
        -> AudioControls::EACEControlType
    {
        switch (type)
        {
        case SteamAudioControlType::Event:
            return AudioControls::eACET_TRIGGER;
            /*
                case SteamAudioControlType::Rtpc:
                    return AudioControls::eACET_RTPC;
                case SteamAudioControlType::Switch:
                    [[fallthrough]];
                case SteamAudioControlType::GameState:
                    return AudioControls::eACET_SWITCH;
                case SteamAudioControlType::Environment:
                    return AudioControls::eACET_ENVIRONMENT;
              */
        case SteamAudioControlType::SoundBank:
            return AudioControls::eACET_PRELOAD;
        }

        return AudioControls::eACET_NUM_TYPES;
    }

    auto AudioSystemEditor_steamaudio::GetCompatibleTypes(
        AudioControls::EACEControlType atlControlType) const -> AudioControls::TImplControlTypeMask
    {
        switch (atlControlType)
        {
        case AudioControls::eACET_TRIGGER:
            return SteamAudioControlType::Event;
        case AudioControls::eACET_RTPC:
            return SteamAudioControlType::Rtpc;
        case AudioControls::eACET_SWITCH:
            return (SteamAudioControlType::Switch | SteamAudioControlType::GameState);
        case AudioControls::eACET_SWITCH_STATE:
            return (
                SteamAudioControlType::Switch | SteamAudioControlTypeNamespace::GameState |
                SteamAudioControlType::Rtpc);
        case AudioControls::eACET_ENVIRONMENT:
            return (SteamAudioControlType::AuxBus | SteamAudioControlType::Rtpc);
        case AudioControls::eACET_PRELOAD:
            return SteamAudioControlType::SoundBank;
        }
        return AudioControls::AUDIO_IMPL_INVALID_TYPE;
    }

    auto AudioSystemEditor_steamaudio::CreateConnectionToControl(
        AudioControls::EACEControlType atlControlType,
        AudioControls::IAudioSystemControl* middlewareControl) -> AudioControls::TConnectionPtr
    {
        if (middlewareControl)
        {
            middlewareControl->SetConnected(true);
            ++m_connectionsByID[middlewareControl->GetId()];

            if (middlewareControl->GetType() == SteamAudioControlType::Rtpc)
            {
                switch (atlControlType)
                {
                case AudioControls::EACEControlType::eACET_RTPC:
                    {
                        return AZStd::make_shared<RtpcConnection>(middlewareControl->GetId());
                    }
                case AudioControls::EACEControlType::eACET_SWITCH_STATE:
                    {
                        return AZStd::make_shared<StateToRtpcConnection>(
                            middlewareControl->GetId());
                    }
                }
            }
            return AZStd::make_shared<AudioControls::IAudioConnection>(middlewareControl->GetId());
        }
        return {};
    }

    auto AudioSystemEditor_steamaudio::CreateConnectionFromXMLNode(
        AZ::rapidxml::xml_node<char>* node,
        AudioControls::EACEControlType atlControlType) -> AudioControls::TConnectionPtr
    {
        if (node)
        {
            AZStd::string_view element(node->name());
            AudioControls::TImplControlType type = TagToType(element);

            if (type != AudioControls::AUDIO_IMPL_INVALID_TYPE)
            {
                AZStd::string name{};
                AZStd::string_view localized{};
                AZStd::optional<AZStd::string> soundFile{};

                if (auto nameAttr =
                        node->first_attribute(SteamAudio::XmlTags::NameAttribute, 0, false);
                    nameAttr != nullptr)
                {
                    name = nameAttr->value();
                }

                if (auto localizedAttr =
                        node->first_attribute(XmlTags::LocalizedAttribute, 0, false))
                {
                    localized = localizedAttr->value();
                }

                if (auto soundFileAttr =
                        node->first_attribute(XmlTags::SoundFileAttribute, 0, false);
                    soundFileAttr != nullptr)
                {
                    soundFile.emplace(soundFileAttr->value());
                }

                bool const isLocalized = AZ::StringFunc::Equal(localized, "true");

                // If the control wasn't found, create a placeholder.
                // We want to see that connection even if it's not in the
                // middleware. User could be viewing the editor without a
                // middleware project.
                AudioControls::IAudioSystemControl* control = GetControlByName(name, isLocalized);

                if (!control)
                {
                    control = CreateControl(AudioControls::SControlDef(name, type));
                    if (control)
                    {
                        control->SetPlaceholder(true);
                        control->SetLocalized(isLocalized);
                    }
                }

                if (type == SteamAudioControlType::Event)
                {
                    auto* steamaudioControl = static_cast<AudioSystemControl_steamaudio*>(control);
                    soundFile.has_value() ? steamaudioControl->SetSoundFile(soundFile.value())
                                          : void();
                }

                // If it's a switch we connect to one of the states within the
                // switch
                if (type == SteamAudioControlType::SwitchGroup ||
                    type == SteamAudioControlType::GameStateGroup)
                {
                    if (auto childNode = node->first_node(); childNode != nullptr)
                    {
                        AZStd::string childName{};
                        if (auto childNameAttr =
                                childNode->first_attribute(XmlTags::NameAttribute, 0, false);
                            childNameAttr != nullptr)
                        {
                            childName = childNameAttr->value();
                        }

                        AudioControls::IAudioSystemControl* childControl =
                            GetControlByName(childName, false, control);
                        if (!childControl)
                        {
                            childControl = CreateControl(AudioControls::SControlDef(
                                childName,
                                type == SteamAudioControlType::SwitchGroup
                                    ? SteamAudioControlType::Switch
                                    : SteamAudioControlType::GameState,
                                false,
                                control));
                        }
                        control = childControl;
                    }
                }

                if (control)
                {
                    control->SetConnected(true);
                    m_connectionsByID[control->GetId()];

                    if (type == SteamAudioControlType::Rtpc)
                    {
                        switch (atlControlType)
                        {
                        case AudioControls::EACEControlType::eACET_RTPC:
                            {
                                RtpcConnectionPtr connection{ AZStd::make_shared<RtpcConnection>(
                                    control->GetId()) };
                                float mult = 1.0f;
                                float shift = 0.0f;

                                if (auto multAttr = node->first_attribute(
                                        XmlTags::MultiplierAttribute, 0, false);
                                    multAttr != nullptr)
                                {
                                    mult = AZStd::stof(AZStd::string(multAttr->value()));
                                    if (auto shiftAttr = node->first_attribute(
                                            XmlTags::ShiftAttribute, 0, false);
                                        shiftAttr != nullptr)
                                    {
                                        shift = AZStd::stof(AZStd::string(shiftAttr->value()));
                                    }

                                    connection->m_mult = mult;
                                    connection->m_shift = shift;
                                    return connection;
                                }
                            }

                        case AudioControls::EACEControlType::eACET_SWITCH_STATE:
                            {
                                StateConnectionPtr connection =
                                    AZStd::make_shared<StateToRtpcConnection>(control->GetId());

                                float value{};
                                if (auto valueAttr =
                                        node->first_attribute(XmlTags::ValueAttribute, 0, false);
                                    valueAttr != nullptr)
                                {
                                    value = AZStd::stof(AZStd::string(valueAttr->value()));
                                }

                                connection->m_value = value;
                                return connection;
                            }
                        case AudioControls::EACEControlType::eACET_ENVIRONMENT:
                            {
                                return AZStd::make_shared<AudioControls::IAudioConnection>(
                                    control->GetId());
                            }
                        }
                    }
                    else
                    {
                        return AZStd::make_shared<AudioControls::IAudioConnection>(
                            control->GetId());
                    }
                }
            }
        }
        return {};
    }

    auto AudioSystemEditor_steamaudio::CreateXMLNodeFromConnection(
        AudioControls::TConnectionPtr const connection,
        AudioControls::EACEControlType const atlControlType) -> AZ::rapidxml::xml_node<char>*
    {
        AudioControls::IAudioSystemControl const* control = GetControl(connection->GetID());
        if (control)
        {
            AudioControls::XmlAllocator& xmlAllocator(AudioControls::s_xmlAllocator);

            switch (control->GetType())
            {
            case SteamAudioControlType::Switch:
                [[fallthrough]];
            case SteamAudioControlType::SwitchGroup:
                [[fallthrough]];
            case SteamAudioControlType::GameState:
                [[fallthrough]];
            case SteamAudioControlType::GameStateGroup:
                {
                    AudioControls::IAudioSystemControl const* parent = control->GetParent();
                    if (parent)
                    {
                        AZStd::string_view parentType = TypeToTag(parent->GetType());
                        auto switchNode = xmlAllocator.allocate_node(
                            AZ::rapidxml::node_element,
                            xmlAllocator.allocate_string(parentType.data()));

                        auto switchNameAttr = xmlAllocator.allocate_attribute(
                            SteamAudio::XmlTags::NameAttribute,
                            xmlAllocator.allocate_string(parent->GetName().c_str()));

                        auto stateNode = xmlAllocator.allocate_node(
                            AZ::rapidxml::node_element, SteamAudio::XmlTags::ValueAttribute);

                        auto stateNameAttr = xmlAllocator.allocate_attribute(
                            SteamAudio::XmlTags::NameAttribute,
                            xmlAllocator.allocate_string(control->GetName().c_str()));

                        switchNode->append_attribute(switchNameAttr);
                        stateNode->append_attribute(stateNameAttr);
                        switchNode->append_node(stateNode);
                        return switchNode;
                    }
                    break;
                }

            case SteamAudioControlType::Rtpc:
                {
                    auto connectionNode = xmlAllocator.allocate_node(
                        AZ::rapidxml::node_element,
                        xmlAllocator.allocate_string(TypeToTag(control->GetType()).data()));

                    auto nameAttr = xmlAllocator.allocate_attribute(
                        SteamAudio::XmlTags::NameAttribute,
                        xmlAllocator.allocate_string(control->GetName().c_str()));

                    connectionNode->append_attribute(nameAttr);

                    if (atlControlType == AudioControls::eACET_RTPC)
                    {
                        AZStd::shared_ptr<RtpcConnection const> rtpcConnection =
                            AZStd::static_pointer_cast<RtpcConnection const>(connection);
                        if (rtpcConnection->m_mult != 1.f)
                        {
                            auto multAttr = xmlAllocator.allocate_attribute(
                                SteamAudio::XmlTags::MultiplierAttribute,
                                xmlAllocator.allocate_string(
                                    AZStd::to_string(rtpcConnection->m_mult).c_str()));

                            connectionNode->append_attribute(multAttr);
                        }

                        if (rtpcConnection->m_shift != 0.f)
                        {
                            auto shiftAttr = xmlAllocator.allocate_attribute(
                                SteamAudio::XmlTags::ShiftAttribute,
                                xmlAllocator.allocate_string(
                                    AZStd::to_string(rtpcConnection->m_shift).c_str()));

                            connectionNode->append_attribute(shiftAttr);
                        }
                    }
                    else if (atlControlType == AudioControls::eACET_SWITCH_STATE)
                    {
                        AZStd::shared_ptr<StateToRtpcConnection const> stateConnection =
                            AZStd::static_pointer_cast<StateToRtpcConnection const>(connection);

                        auto valueAttr = xmlAllocator.allocate_attribute(
                            SteamAudio::XmlTags::ValueAttribute,
                            xmlAllocator.allocate_string(
                                AZStd::to_string(stateConnection->m_value).c_str()));

                        connectionNode->append_attribute(valueAttr);
                    }

                    return connectionNode;
                }

            case SteamAudioControlType::Event:
                [[fallthrough]];
            case SteamAudioControlType::AuxBus:
                {
                    auto connectionNode = xmlAllocator.allocate_node(
                        AZ::rapidxml::node_element,
                        xmlAllocator.allocate_string(TypeToTag(control->GetType()).data()));

                    auto nameAttr = xmlAllocator.allocate_attribute(
                        SteamAudio::XmlTags::NameAttribute,
                        xmlAllocator.allocate_string(control->GetName().c_str()));

                    auto const* const saControl =
                        static_cast<AudioSystemControl_steamaudio const*>(control);
                    auto soundAttr = xmlAllocator.allocate_attribute(
                        SteamAudio::XmlTags::SoundFileAttribute,
                        xmlAllocator.allocate_string(saControl->GetSoundFile()->c_str()));

                    connectionNode->append_attribute(nameAttr);
                    connectionNode->append_attribute(soundAttr);

                    return connectionNode;
                }

            case SteamAudioControlType::SoundBank:
                {
                    auto connectionNode = xmlAllocator.allocate_node(
                        AZ::rapidxml::node_element,
                        xmlAllocator.allocate_string(TypeToTag(control->GetType()).data()));

                    auto nameAttr = xmlAllocator.allocate_attribute(
                        SteamAudio::XmlTags::NameAttribute,
                        xmlAllocator.allocate_string(control->GetName().c_str()));

                    connectionNode->append_attribute(nameAttr);

                    if (control->IsLocalized())
                    {
                        auto locAttr = xmlAllocator.allocate_attribute(
                            SteamAudio::XmlTags::LocalizedAttribute,
                            xmlAllocator.allocate_string("true"));

                        connectionNode->append_attribute(locAttr);
                    }

                    return connectionNode;
                }
            }
        }
        return nullptr;
    }

    auto AudioSystemEditor_steamaudio::GetTypeIcon(AudioControls::TImplControlType type) const
        -> AZStd::string_view const
    {
        switch (type)
        {
        case SteamAudioControlType::Event:
            return ":/SteamAudio/trigger_icon.svg";
        default:
            return {};
        };
    }

    auto AudioSystemEditor_steamaudio::GetTypeIconSelected(
        AudioControls::TImplControlType /*type*/) const -> AZStd::string_view const
    {
        return {};
    }

    auto AudioSystemEditor_steamaudio::GetName() const -> AZStd::string
    {
        return "O3DE-SteamAudio";
    }
    auto AudioSystemEditor_steamaudio::GetDataPath() const -> AZ::IO::FixedMaxPath
    {
        return { RuntimePath };
    }

    void AudioSystemEditor_steamaudio::ConnectionRemoved(
        AudioControls::IAudioSystemControl* control)
    {
        int connectionCount = m_connectionsByID[control->GetId()] - 1;
        if (connectionCount <= 0)
        {
            connectionCount = 0;
            control->SetConnected(false);
        }
        m_connectionsByID[control->GetId()] = connectionCount;
    }

    auto AudioSystemEditor_steamaudio::GetControlByName(
        AZStd::string name,
        bool isLocalized,
        AudioControls::IAudioSystemControl* parent) const -> AudioControls::IAudioSystemControl*
    {
        if (parent)
        {
            AZ::StringFunc::Path::Join(parent->GetName().c_str(), name.c_str(), name);
        }

        if (isLocalized)
        {
            auto o3deProjectRelativePath{ AZ::IO::Path{ m_loader.GetLocalizationFolder() } /
                                          name.c_str() };
            name = o3deProjectRelativePath.Native();
        }

        return GetControl(GetID(name));
    }

    auto AudioSystemEditor_steamaudio::GetID(AZStd::string_view const name) const
        -> AudioControls::CID
    {
        return Audio::AudioStringToID<AudioControls::CID>(name.data());
    }

    void AudioSystemEditor_steamaudio::UpdateConnectedStatus()
    {
        for (auto const& idCountPair : m_connectionsByID)
        {
            if (idCountPair.second > 0)
            {
                AudioControls::IAudioSystemControl* control = GetControl(idCountPair.first);
                if (control)
                {
                    control->SetConnected(true);
                }
            }
        }
    }

} // namespace SteamAudio
