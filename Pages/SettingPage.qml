import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtMultimedia

Page {
    id: settingPage

    // 音频播放器用于试听
    MediaPlayer {
        id: previewPlayer
        audioOutput: AudioOutput {}
    }

    background: Rectangle {
        color: "#f5f5f5"
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 20
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            // 页面标题
            Label {
                text: "个性化定制"
                font.family: "Microsoft YaHei"
                font.pixelSize: 28
                font.bold: true
                color: "#333333"
                Layout.bottomMargin: 10
            }

            // 用户设置卡片
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: userColumn.implicitHeight + 40
                radius: 12
                color: "#FFFFFF"
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    id: userColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: "用户设置"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#333333"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        Label {
                            text: "你的名字"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                            Layout.preferredWidth: 100
                        }

                        TextField {
                            id: userNameField
                            Layout.fillWidth: true
                            placeholderText: "输入你的名字"
                            font.family: "Microsoft YaHei"
                            text: configMgr.userName
                            onTextChanged: configMgr.userName = text
                        }
                    }

                    Label {
                        text: "关于你（可选）"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 14
                        color: "#666666"
                    }

                    TextArea {
                        id: userDescField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        placeholderText: "介绍一下你自己，让AI更了解你..."
                        font.family: "Microsoft YaHei"
                        wrapMode: TextArea.Wrap
                        text: configMgr.userDescription
                        onTextChanged: configMgr.userDescription = text
                        background: Rectangle {
                            color: "#F5F5F5"
                            radius: 8
                            border.color: userDescField.activeFocus ? "#007AFF" : "#E0E0E0"
                        }
                    }
                }
            }

            // AI角色设置卡片
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: aiColumn.implicitHeight + 40
                radius: 12
                color: "#FFFFFF"
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    id: aiColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: "AI角色设置"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#333333"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        Label {
                            text: "AI名字"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                            Layout.preferredWidth: 100
                        }

                        TextField {
                            id: aiNameField
                            Layout.fillWidth: true
                            placeholderText: "给AI起个名字"
                            font.family: "Microsoft YaHei"
                            text: configMgr.aiName
                            onTextChanged: configMgr.aiName = text
                        }
                    }

                    Label {
                        text: "AI性格描述"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 14
                        color: "#666666"
                    }

                    TextArea {
                        id: aiPersonalityField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        placeholderText: "描述AI的性格特点、说话风格等..."
                        font.family: "Microsoft YaHei"
                        wrapMode: TextArea.Wrap
                        text: configMgr.aiPersonality
                        onTextChanged: configMgr.aiPersonality = text
                        background: Rectangle {
                            color: "#F5F5F5"
                            radius: 8
                            border.color: aiPersonalityField.activeFocus ? "#007AFF" : "#E0E0E0"
                        }
                    }

                    Label {
                        text: "自定义系统提示词（高级）"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 14
                        color: "#666666"
                    }

                    TextArea {
                        id: systemPromptField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                        placeholderText: "留空则使用默认提示词模板。自定义提示词将完全替代默认模板..."
                        font.family: "Microsoft YaHei"
                        wrapMode: TextArea.Wrap
                        text: configMgr.systemPrompt
                        onTextChanged: configMgr.systemPrompt = text
                        background: Rectangle {
                            color: "#F5F5F5"
                            radius: 8
                            border.color: systemPromptField.activeFocus ? "#007AFF" : "#E0E0E0"
                        }
                    }
                }
            }

            // 服务器配置卡片
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: serverColumn.implicitHeight + 40
                radius: 12
                color: "#FFFFFF"
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    id: serverColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: "服务器配置"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#333333"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 20
                        rowSpacing: 12

                        Label {
                            text: "LLM服务器"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                        }
                        TextField {
                            id: llmUrlField
                            Layout.fillWidth: true
                            placeholderText: "http://localhost:11434/api/chat"
                            font.family: "Microsoft YaHei"
                            text: configMgr.llmServerUrl
                            onTextChanged: configMgr.llmServerUrl = text
                        }

                        Label {
                            text: "LLM模型"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                        }
                        TextField {
                            id: llmModelField
                            Layout.fillWidth: true
                            placeholderText: "qwen3.5:0.8b"
                            font.family: "Microsoft YaHei"
                            text: configMgr.llmModelName
                            onTextChanged: configMgr.llmModelName = text
                        }
                    }
                }
            }

            // 讯飞 TTS 设置卡片
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: iflytekColumn.implicitHeight + 40
                radius: 12
                color: "#FFFFFF"
                border.color: "#E0E0E0"
                border.width: 1

                ColumnLayout {
                    id: iflytekColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: "讯飞 TTS 设置（语音合成）"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#333333"
                    }

                    // 凭证
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20
                        Label {
                            text: "APPID"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                            Layout.preferredWidth: 100
                        }
                        TextField {
                            id: iflytekAppIdField
                            Layout.fillWidth: true
                            placeholderText: "讯飞开放平台 APPID"
                            font.family: "Microsoft YaHei"
                            text: configMgr.iflytekAppId
                            onTextChanged: configMgr.iflytekAppId = text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20
                        Label {
                            text: "APIKey"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                            Layout.preferredWidth: 100
                        }
                        TextField {
                            id: iflytekApiKeyField
                            Layout.fillWidth: true
                            placeholderText: "讯飞开放平台 APIKey"
                            font.family: "Microsoft YaHei"
                            text: configMgr.iflytekApiKey
                            onTextChanged: configMgr.iflytekApiKey = text
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20
                        Label {
                            text: "APISecret"
                            font.family: "Microsoft YaHei"
                            font.pixelSize: 14
                            color: "#666666"
                            Layout.preferredWidth: 100
                        }
                        TextField {
                            id: iflytekApiSecretField
                            Layout.fillWidth: true
                            placeholderText: "讯飞开放平台 APISecret"
                            font.family: "Microsoft YaHei"
                            echoMode: TextInput.Password
                            text: configMgr.iflytekApiSecret
                            onTextChanged: configMgr.iflytekApiSecret = text
                        }
                        Button {
                            text: iflytekApiSecretField.echoMode === TextInput.Password ? "显示" : "隐藏"
                            font.family: "Microsoft YaHei"
                            Layout.preferredWidth: 60
                            onClicked: {
                                iflytekApiSecretField.echoMode =
                                    iflytekApiSecretField.echoMode === TextInput.Password ?
                                    TextInput.Normal : TextInput.Password
                            }
                        }
                    }

                    // 发音人
                    Label {
                        text: "发音人 (vcn)"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 14
                        color: "#666666"
                        Layout.topMargin: 8
                    }

                    ComboBox {
                        id: iflytekVoiceCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        editable: true
                        model: ListModel {
                            id: voiceListModel
                            ListElement { text: "青年女声 (xiaoyan)"; vcn: "xiaoyan" }
                            ListElement { text: "萝莉 (aisjiqiux)"; vcn: "aisjiqiux" }
                            ListElement { text: "磁性男声 (aisxping)"; vcn: "aisxping" }
                            ListElement { text: "东北女声 (aisjinger)"; vcn: "aisjinger" }
                            ListElement { text: "童声 (aisbabyxu)"; vcn: "aisbabyxu" }
                        }
                        textRole: "text"

                        Component.onCompleted: {
                            var vcn = configMgr.iflytekVoiceName
                            for (var i = 0; i < voiceListModel.count; i++) {
                                if (voiceListModel.get(i).vcn === vcn) {
                                    currentIndex = i
                                    return
                                }
                            }
                            if (vcn !== "") {
                                voiceListModel.append({text: vcn + " (自定义)", vcn: vcn})
                                currentIndex = voiceListModel.count - 1
                            }
                        }

                        onActivated: {
                            if (currentIndex >= 0 && currentIndex < voiceListModel.count) {
                                configMgr.iflytekVoiceName = voiceListModel.get(currentIndex).vcn
                            }
                        }

                        onAccepted: {
                            var txt = editText.trim()
                            if (txt !== "") {
                                configMgr.iflytekVoiceName = txt
                            }
                        }
                    }

                    // 语速 / 音调 / 音量 滑块
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 3
                        columnSpacing: 20
                        rowSpacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Label {
                                text: "语速: " + iflytekSpeedSlider.value
                                font.family: "Microsoft YaHei"
                                font.pixelSize: 12
                                color: "#666666"
                            }
                            Slider {
                                id: iflytekSpeedSlider
                                Layout.fillWidth: true
                                from: 0; to: 100; stepSize: 1
                                value: configMgr.iflytekSpeed
                                onValueChanged: configMgr.iflytekSpeed = Math.round(value)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Label {
                                text: "音调: " + iflytekPitchSlider.value
                                font.family: "Microsoft YaHei"
                                font.pixelSize: 12
                                color: "#666666"
                            }
                            Slider {
                                id: iflytekPitchSlider
                                Layout.fillWidth: true
                                from: 0; to: 100; stepSize: 1
                                value: configMgr.iflytekPitch
                                onValueChanged: configMgr.iflytekPitch = Math.round(value)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Label {
                                text: "音量: " + iflytekVolumeSlider.value
                                font.family: "Microsoft YaHei"
                                font.pixelSize: 12
                                color: "#666666"
                            }
                            Slider {
                                id: iflytekVolumeSlider
                                Layout.fillWidth: true
                                from: 0; to: 100; stepSize: 1
                                value: configMgr.iflytekVolume
                                onValueChanged: configMgr.iflytekVolume = Math.round(value)
                            }
                        }
                    }

                    // 测试按钮
                    Button {
                        text: "测试发音"
                        font.family: "Microsoft YaHei"
                        Layout.alignment: Qt.AlignLeft
                        onClicked: {
                            if (typeof app !== "undefined" && app.testTts) {
                                app.testTts()
                            }
                        }
                    }

                    Label {
                        text: "提示：凭据在 console.xfyun.cn 获取。发音人可输入自定义名称。每次 AI 回复都会自动合成语音。"
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 11
                        color: "#999999"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }

            // 操作按钮
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 20

                Item { Layout.fillWidth: true }

                Button {
                    text: "重置为默认"
                    font.family: "Microsoft YaHei"
                    onClicked: {
                        resetDialog.open()
                    }
                }

                Button {
                    text: "保存设置"
                    font.family: "Microsoft YaHei"
                    highlighted: true
                    onClicked: {
                        configMgr.saveConfig()
                        saveSuccessAnimation.start()
                    }
                }
            }

            // 保存成功提示
            Label {
                id: saveSuccessLabel
                text: "✓ 设置已保存"
                font.family: "Microsoft YaHei"
                font.pixelSize: 14
                color: "#4CAF50"
                opacity: 0
                Layout.alignment: Qt.AlignHCenter

                SequentialAnimation {
                    id: saveSuccessAnimation
                    NumberAnimation { target: saveSuccessLabel; property: "opacity"; to: 1; duration: 200 }
                    PauseAnimation { duration: 2000 }
                    NumberAnimation { target: saveSuccessLabel; property: "opacity"; to: 0; duration: 500 }
                }
            }

            Item { Layout.preferredHeight: 40 }
        }
    }

    // 重置确认对话框
    Dialog {
        id: resetDialog
        title: "确认重置"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: "确定要将所有设置重置为默认值吗？"
            font.family: "Microsoft YaHei"
        }

        onAccepted: {
            configMgr.resetToDefault()
        }
    }
}
