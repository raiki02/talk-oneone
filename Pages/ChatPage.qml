import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Components"
import "../js/addListModel.js" as AddListModel

Page {
    id: page

    property bool isWaitingResponse: false
    property bool isPlayingTts: false

    background: Rectangle {
        color: "#f8f9fa"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 对话列表
        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: false
            model: ChatMessageListModel {}
            spacing: 16
            clip: true
            leftMargin: 20
            rightMargin: 20
            topMargin: 20
            bottomMargin: 20

            // 自动滚动到底部
            onCountChanged: {
                Qt.callLater(function() {
                    listView.positionViewAtEnd()
                })
            }

            // 添加滚动动画
            Behavior on contentY {
                NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
            }

            delegate: Item {
                id: delegateItem
                width: listView.width - 40
                height: messageColumn.height

                // 入场动画
                opacity: 0
                Component.onCompleted: {
                    enterAnimation.start()
                }

                SequentialAnimation {
                    id: enterAnimation
                    NumberAnimation {
                        target: delegateItem
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.OutQuad
                    }
                }

                NumberAnimation {
                    id: slideAnimation
                    target: messageColumn
                    property: "x"
                    from: model.role === "User" ? 50 : -50
                    to: 0
                    duration: 300
                    easing.type: Easing.OutBack
                    running: true
                }

                Column {
                    id: messageColumn
                    width: parent.width
                    spacing: 4

                    // 角色名称
                    Label {
                        id: nameLabel
                        text: model.role === "User" ? "你" : configMgr.aiName
                        font.family: "Microsoft YaHei"
                        font.pointSize: 9
                        color: "#666666"
                        visible: model.role !== ""
                        anchors.left: model.role === "User" ? undefined : parent.left
                        anchors.right: model.role === "User" ? parent.right : undefined
                        anchors.leftMargin: model.role === "User" ? 0 : 10
                        anchors.rightMargin: model.role === "User" ? 10 : 0
                    }

                    // 消息气泡
                    Rectangle {
                        id: messageRect
                        width: Math.min(messageLabel.implicitWidth + 24, listView.width * 0.65)
                        height: messageLabel.implicitHeight + 20
                        radius: 16

                        property string role: model.role
                        color: role === "User" ? "#007AFF" : "#FFFFFF"
                        border.color: role === "User" ? "transparent" : "#E5E5EA"
                        border.width: role === "User" ? 0 : 1

                        anchors.left: role === "User" ? undefined : parent.left
                        anchors.right: role === "User" ? parent.right : undefined

                        // 悬停效果
                        scale: messageMouseArea.containsMouse ? 1.02 : 1.0
                        Behavior on scale {
                            NumberAnimation { duration: 150 }
                        }

                        Label {
                            id: messageLabel
                            anchors.centerIn: parent
                            width: Math.min(implicitWidth, listView.width * 0.65 - 24)
                            text: model.context
                            font.family: "Microsoft YaHei"
                            font.pointSize: 12
                            color: messageRect.role === "User" ? "#FFFFFF" : "#333333"
                            wrapMode: Text.Wrap
                            lineHeight: 1.3
                        }

                        MouseArea {
                            id: messageMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                // 复制到剪贴板的功能可以在这里添加
                            }
                        }
                    }

                    // 时间戳
                    Label {
                        id: timestampLabel
                        text: model.timestamp ? new Date(model.timestamp).toLocaleTimeString(Qt.locale(), "hh:mm") : ""
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 10
                        color: "#999999"
                        anchors.left: model.role === "User" ? undefined : parent.left
                        anchors.right: model.role === "User" ? parent.right : undefined
                        anchors.leftMargin: model.role === "User" ? 0 : 10
                        anchors.rightMargin: model.role === "User" ? 10 : 0
                        visible: model.timestamp !== undefined && model.timestamp !== ""
                    }
                }
            }

            // 加载/等待回复指示器 or TTS 播放指示器
            footer: Item {
                width: listView.width - 40
                height: (isWaitingResponse || isPlayingTts) ? 60 : 0
                visible: isWaitingResponse || isPlayingTts

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Label {
                        text: isPlayingTts ? (configMgr.aiName + " 正在说话") : (configMgr.aiName + " 正在输入")
                        font.family: "Microsoft YaHei"
                        font.pixelSize: 13
                        color: "#666666"
                    }

                    // 动画点
                    Row {
                        spacing: 4
                        anchors.verticalCenter: parent.verticalCenter

                        Repeater {
                            model: 3
                            Rectangle {
                                width: 6
                                height: 6
                                radius: 3
                                color: "#007AFF"

                                SequentialAnimation on opacity {
                                    running: isWaitingResponse || isPlayingTts
                                    loops: Animation.Infinite
                                    PauseAnimation { duration: index * 200 }
                                    NumberAnimation { from: 0.3; to: 1; duration: 400 }
                                    NumberAnimation { from: 1; to: 0.3; duration: 400 }
                                }

                                SequentialAnimation on y {
                                    running: isWaitingResponse || isPlayingTts
                                    loops: Animation.Infinite
                                    PauseAnimation { duration: index * 200 }
                                    NumberAnimation { from: 0; to: -4; duration: 300; easing.type: Easing.OutQuad }
                                    NumberAnimation { from: -4; to: 0; duration: 300; easing.type: Easing.InQuad }
                                }
                            }
                        }
                    }
                }
            }
        }

        // 欢迎区域和输入框
        Column {
            Layout.alignment: Qt.AlignCenter
            bottomPadding: 20
            spacing: 50

            // 欢迎语
            Label {
                id: greetLabel
                visible: true
                anchors.horizontalCenter: parent.horizontalCenter
                text: "你好，我能为你做点什么"
                font.family: "Microsoft YaHei"
                font.pixelSize: 28
                color: "#333333"

                // 淡入动画
                opacity: 0
                Component.onCompleted: {
                    greetFadeIn.start()
                }

                NumberAnimation {
                    id: greetFadeIn
                    target: greetLabel
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 800
                    easing.type: Easing.OutQuad
                }
            }

            // 输入框容器
            Rectangle {
                id: inputContainer
                radius: 24
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(750, page.width - 40)
                height: chatColumn.implicitHeight
                color: "#FFFFFF"
                border.color: textArea.activeFocus ? "#007AFF" : "#E5E5EA"
                border.width: textArea.activeFocus ? 2 : 1

                // 聚焦时的阴影效果
                layer.enabled: true

                Behavior on border.color {
                    ColorAnimation { duration: 200 }
                }

                Column {
                    id: chatColumn
                    anchors.fill: parent
                    spacing: 10
                    bottomPadding: 8
                    topPadding: 8

                    ScrollView {
                        id: sendScrollView
                        width: parent.width
                        height: Math.min(textArea.contentHeight + 30, 200)
                        clip: true

                        TextArea {
                            id: textArea
                            leftPadding: 20
                            topPadding: 15
                            rightPadding: 20
                            width: parent.width
                            placeholderText: "输入消息..."
                            placeholderTextColor: "#999999"
                            font.family: "Microsoft YaHei"
                            font.pointSize: 12
                            wrapMode: TextEdit.Wrap

                            background: Rectangle {
                                color: "transparent"
                            }

                            onLengthChanged: {
                                sendButton.enabled = length > 0
                            }

                            // 回车发送
                            Keys.onReturnPressed: function(event) {
                                if (!(event.modifiers & Qt.ShiftModifier) && textArea.text.trim() !== "") {
                                    sendMessage()
                                    event.accepted = true
                                } else {
                                    event.accepted = false
                                }
                            }
                        }
                    }

                    RowLayout {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width - 20
                        spacing: 15

                        Item { Layout.fillWidth: true }

                        // 发送按钮
                        RoundButton {
                            id: sendButton
                            Layout.rightMargin: 10
                            width: 40
                            height: 40
                            icon.source: "qrc:/img/icon/arrow-narrow-right.svg"
                            icon.width: 20
                            icon.height: 20
                            enabled: false

                            ToolTip.visible: hovered
                            ToolTip.text: "发送消息"

                            background: Rectangle {
                                radius: 20
                                color: sendButton.enabled ?
                                       (sendButton.pressed ? "#0056B3" : "#007AFF") :
                                       "#E0E0E0"

                                Behavior on color {
                                    ColorAnimation { duration: 150 }
                                }
                            }

                            // 按钮缩放动画
                            scale: sendButton.pressed ? 0.95 : 1.0
                            Behavior on scale {
                                NumberAnimation { duration: 100 }
                            }

                            onClicked: {
                                sendMessage()
                            }
                        }
                    }
                }
            }
        }
    }

    // 服务器状态警告对话框
    Dialog {
        id: serverStatusDialog
        title: "服务状态异常"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok

        ColumnLayout {
            spacing: 10
            Label {
                text: "Ollama 服务未连接，无法发送消息。\n\n请检查 Ollama 服务是否已启动。"
                font.family: "Microsoft YaHei"
                visible: app && !app.ollamaOnline
            }
            Label {
                text: app && !app.ttsOnline ? "讯飞 TTS 服务未配置，消息将没有语音播报。" : ""
                font.family: "Microsoft YaHei"
                color: "#e67e22"
                visible: app && app.ollamaOnline && !app.ttsOnline
            }
        }
    }

    // 发送消息函数
    function sendMessage() {
        if (textArea.text.trim() === "") return

        // 检查 LLM 服务器状态
        if (app && !app.ollamaOnline) {
            serverStatusDialog.open()
            return
        }

        greetLabel.visible = false
        listView.visible = true
        isWaitingResponse = true

        AddListModel.addChatMessage(listView.model, textArea.text)
        app.sendChatMessage(textArea.text)
        textArea.clear()
    }

    // 加载历史消息的函数
    function loadHistory() {
        if (!app) return

        // 清空现有消息
        listView.model.clear()

        var history = app.chatHistory
        if (history.length > 0) {
            greetLabel.visible = false
            listView.visible = true
            for (var i = 0; i < history.length; i++) {
                var msg = history[i]
                var role = msg.role || "User"
                AddListModel.addChatMessage(listView.model, msg.content, role)
            }
            // 滚动到底部
            Qt.callLater(function() {
                listView.positionViewAtEnd()
            })
        } else {
            greetLabel.visible = true
            listView.visible = false
        }
    }

    Component.onCompleted: {
        // 加载历史消息
        loadHistory()
    }

    // 信号连接
    Connections {
        target: app
        function onSignal_receive_llm_stream(delta, isDone) {
            if (isDone) {
                // 流式完成 → 开始 TTS 播放
                isWaitingResponse = false
                isPlayingTts = true
            } else if (delta) {
                // 增量 token
                if (isWaitingResponse) {
                    AddListModel.appendToLast(listView.model, delta, "Assistant")
                } else {
                    // 第一个 chunk，创建新的 Assistant 消息
                    AddListModel.addChatMessage(listView.model, delta, "Assistant")
                    isWaitingResponse = true
                }
            }
        }

        // TTS 播放完成
        function onSignal_tts_playback_finished() {
            isPlayingTts = false
        }

        // 监听历史记录变化（仅在切换对话时刷新）
        function onCurrentConversationIdChanged() {
            if (page.visible) {
                loadHistory()
            }
        }
    }
}
