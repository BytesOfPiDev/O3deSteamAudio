#pragma once

#include <AzTest/AzTest.h>

#include "IAudioSystemEditor.h"

class MockAudioSystemEditor : public AudioControls::IAudioSystemEditor
{
public:
    MockAudioSystemEditor()
    {
    }
    ~MockAudioSystemEditor() override{};

    MOCK_METHOD0(Reload, void(void));
    MOCK_METHOD1(
        CreateControl, AudioControls::IAudioSystemControl*(AudioControls::SControlDef const&));
    MOCK_METHOD0(GetRoot, AudioControls::IAudioSystemControl*(void));
    MOCK_CONST_METHOD1(GetControl, AudioControls::IAudioSystemControl*(AudioControls::CID));
    MOCK_CONST_METHOD1(
        ImplTypeToATLType, AudioControls::EACEControlType(AudioControls::TImplControlType));
    MOCK_CONST_METHOD1(
        GetCompatibleTypes, AudioControls::TImplControlTypeMask(AudioControls::EACEControlType));

    MOCK_METHOD2(
        CreateConnectionToControl,
        AudioControls::TConnectionPtr(
            AudioControls::EACEControlType atlControlType,
            AudioControls::IAudioSystemControl* middlewareControl));

    MOCK_METHOD2(
        CreateConnectionFromXMLNode,
        AudioControls::TConnectionPtr(
            AZ::rapidxml::xml_node<char>* node, AudioControls::EACEControlType atlControlType));

    MOCK_METHOD2(
        CreateXMLNodeFromConnection,
        AZ::rapidxml::xml_node<char>*(
            AudioControls::TConnectionPtr const connection,
            AudioControls::EACEControlType const atlControlType));

    MOCK_CONST_METHOD1(GetTypeIcon, AZStd::string_view const(AudioControls::TImplControlType type));

    MOCK_CONST_METHOD1(
        GetTypeIconSelected, AZStd::string_view const(AudioControls::TImplControlType type));

    MOCK_CONST_METHOD0(GetName, AZStd::string(void));
    MOCK_CONST_METHOD0(GetDataPath, AZ::IO::FixedMaxPath(void));
    MOCK_METHOD0(DataSaved, void(void));
    MOCK_METHOD1(ConnectionRemoved, void(AudioControls::IAudioSystemControl* control));
};
