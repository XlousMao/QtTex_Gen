import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

pragma ComponentBehavior: Bound

ApplicationWindow  {
    visible:true
    width: 900
    height: 600
    title: qsTr("Anki Math Card Converter (QtTex_Gen)")
    color: "#F3F4F6"

    font.family: "Segoe UI, Microsoft YaHei, sans-serif"
    font.pixelSize: 13

    // 页面加载完成后自动执行
    Component.onCompleted: {
        appController.checkAnkiConnection()
    }

    function pickPreferredField(fields, prefers) {
        for (var i = 0; i < prefers.length; ++i) {
            var p = prefers[i].toLowerCase()
            for (var j = 0; j < fields.length; ++j) {
                if ((fields[j] + "").toLowerCase() === p) {
                    return fields[j]
                }
            }
        }
        return fields.length > 0 ? fields[0] : ""
    }

    // 1. 定义打开文件对话框
    FileDialog {
        id: fileDialog
        title: "Please choose a Markdown file"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Markdown files (*.md)", "All files (*)"]
        onAccepted: {
            appController.openMarkdownFile(fileDialog.currentFile)
        }
    }

    // 2. 定义保存文件对话框
    FileDialog {
        id: saveDialog
        title: "Please choose where to save"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Markdown files (*.md)", "All files (*)"]
        onAccepted: {
            appController.saveResultFile(saveDialog.currentFile)
        }
    }

    // 3. 预览结果窗口
    Window {
        id: previewWindow
        title: "Conversion Preview"
        width: 600
        height: 500
        color: "#F3F4F6"
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: 12
            color: "#FFFFFF"
            radius: 10
            border.color: "#E5E7EB"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: 10
                TextArea {
                    id: previewArea
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    background: Item {}
                }
            }
        }
    }

    // 顶部：核心按钮功能 + Anki配置区域
    header: ToolBar {
        id: mainHeader
        contentHeight: topLayout.implicitHeight+20
        
background: Rectangle {
            color: "#FFFFFF"
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#E5E7EB"
            }
        }

        ColumnLayout {
            id: topLayout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 10
            spacing: 12
            
            // 基础功能
            RowLayout {
                spacing: 10
                Button { 
                    text: "Open MD File"
                    onClicked: fileDialog.open()
                    background: Rectangle {
                        color: parent.down ? "#E5E7EB" : (parent.hovered ? "#F3F4F6" : "#FFFFFF")
                        radius: 8
                        border.color: "#D1D5DB"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }     
                Button { 
                    text: "Preview Results"
                    onClicked: {
                        appController.processCards()
                        previewWindow.show()
                    }
                    background: Rectangle {
                        color: parent.down ? "#E5E7EB" : (parent.hovered ? "#F3F4F6" : "#FFFFFF")
                        radius: 8
                        border.color: "#D1D5DB"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }
                Item { Layout.fillWidth: true } // 撑开
                Button { 
                    text: "Save File"
                    onClicked: saveDialog.open()
                    background: Rectangle {
                        color: parent.down ? "#E5E7EB" : (parent.hovered ? "#F3F4F6" : "#FFFFFF")
                        radius: 8
                        border.color: "#D1D5DB"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                } 
            }

            // Anki 配置与导入
            RowLayout {
                spacing: 8
                
                Label { text: "Deck:" }
                ComboBox { 
                    id: deckComboBox
                    Layout.fillWidth: true // 变成液态
                    Layout.preferredWidth: 150
                    textRole: "" 
                }

                Label { text: "Model:" }
                ComboBox { 
                    id: modelComboBox
                    Layout.fillWidth: true
                    Layout.preferredWidth: 150
                    textRole: ""
                    onCurrentTextChanged: {
                        if (currentText && currentText.length > 0) {
                            appController.fetchModelFieldNames(currentText)
                        }
                    }
                }
                
                Label { text: "Front Field:" }
                ComboBox {
                    id: frontInput
                    Layout.fillWidth: true
                    Layout.preferredWidth: 120
                    editable: true
                }

                Label { text: "Back Field:" }
                ComboBox {
                    id: backInput
                    Layout.fillWidth: true
                    Layout.preferredWidth: 120
                    editable: true
                }

                

                Button {
                    text: "Import directly to Anki"
                    Layout.preferredHeight: 36
                    onClicked: {
                        if ((modelComboBox.currentText + "").toLowerCase().indexOf("image occlusion") !== -1) {
                            logArea.text += ">>> [ERROR] 当前模板是 Image Occlusion 类型，需要图片与遮罩字段，不能直接用纯文本导入。请改用 Basic/Cloze 模板。\n"
                            return
                        }

                        logArea.text += ">>> Starting Import...\n"
                        appController.importCardsToAnki(deckComboBox.currentText, 
                                                        modelComboBox.currentText, 
                                                        frontInput.editText, 
                                                        backInput.editText)
                    }
                    background: Rectangle {
                        color: parent.down ? "#1D4ED8" : (parent.hovered ? "#3B82F6" : "#2563EB")
                        radius: 8
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // 主体：分栏布局 
    SplitView {
        id:mainSplitView
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Rectangle {
            SplitView.preferredWidth: parent.width / 2
            color: "#FFFFFF"
            radius: 10
            border.color: "#E5E7EB"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: 12
                TextArea {
                    id: leftTextArea
                    placeholderText: "左侧显示打开的Markdown内容..."
                    wrapMode: TextArea.Wrap
                    background: Item {}
                }
            }
        }
        
        Rectangle {
            color: "#FFFFFF"
            radius: 10
            border.color: "#E5E7EB"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: 12
                TextArea {
                    id: logArea
                    readOnly: true
                    placeholderText: "右侧显示转换日志和状态信息..."
                    wrapMode: TextArea.Wrap
                    background: Item {}
                }
            }
        }
    }

    // 底部：状态栏
    footer: ToolBar {
        RowLayout{
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            Label {
                id: rightInfoLabell
                text: "Anki Math Converter v2.0"
                font.italic: true
                color: "gray"
            }


            Item {
                Layout.fillWidth: true
            }
            
           
            Label {
            id: statusLabel
            text: "Ready"
            padding: 5
            color: "black" 
            }
        }
    }

     // ===================================
    // 连接 C++ 信号与 QML
    Connections {
        target: appController
        function onFileLoaded(rawContent) { leftTextArea.text = rawContent; }
        function onProcessCompleted(resultText) { previewArea.text = resultText; }
        function onErrorMessage(msg) { statusLabel.text = msg; }

        function onAnkiConnectionChecked(success, version) {
            if (success) {
                statusLabel.text = "已连接到 Anki (版本 " + version + ")";
                statusLabel.color = "green";
                logArea.text += ">>> [API] Connected to Anki Connect.\n";
                // 自动拉取
                appController.fetchDeckNames();
                appController.fetchModelNames();
            } else {
                statusLabel.text = "未连接到 Anki (请确保 Anki 运行并安装 AnkiConnect)";
                statusLabel.color = "red";
                logArea.text += ">>> [API ERROR] Could not connect to Anki!\n";
            }
        }

        function onAnkiDeckNamesFetched(success, decks) {
            if(success) {
                deckComboBox.model = decks;
                logArea.text += ">>> [API] Decks fetched. Total: " + decks.length + "\n";
            }
        }

        function onAnkiModelNamesFetched(success, models) {
            if(success) {
                modelComboBox.model = models;
                logArea.text += ">>> [API] Note Models fetched. Total: " + models.length + "\n";

                var fillBlankIndex = -1
                var clozeIndex = -1
                var basicIndex = -1
                for (var i = 0; i < models.length; ++i) {
                    var raw = (models[i] + "")
                    var m = raw.toLowerCase()
                    if (fillBlankIndex < 0 && raw.indexOf("填空") !== -1)
                        fillBlankIndex = i
                    if (clozeIndex < 0 && m.indexOf("cloze") !== -1)
                        clozeIndex = i
                    if (basicIndex < 0 && m.indexOf("basic") !== -1)
                        basicIndex = i
                }

                if (fillBlankIndex >= 0) {
                    modelComboBox.currentIndex = fillBlankIndex
                } else if (clozeIndex >= 0) {
                    modelComboBox.currentIndex = clozeIndex
                } else if (basicIndex >= 0) {
                    modelComboBox.currentIndex = basicIndex
                }

                if (modelComboBox.currentText && modelComboBox.currentText.length > 0) {
                    appController.fetchModelFieldNames(modelComboBox.currentText)
                }
            }
        }

        function onAnkiModelFieldNamesFetched(success, fieldNames, errorMsg) {
            if (success) {
                frontInput.model = fieldNames
                backInput.model = fieldNames

                frontInput.editText = pickPreferredField(fieldNames, ["Front", "正面", "Text", "题目"])
                backInput.editText = pickPreferredField(fieldNames, ["Back", "背面", "背面额外", "Extra", "答案"])

                logArea.text += ">>> [API] Model fields: " + fieldNames.join(", ") + "\n"
            } else {
                logArea.text += ">>> [API ERROR] 获取模板字段失败: " + errorMsg + "\n"
            }
        }

        function onAnkiNoteAdded(success, errorMsg) {
            if(success) {
                logArea.text += "- [SUCCESS] " + errorMsg + "\n";
            } else {
                logArea.text += "- [FAILED] " + errorMsg + "\n";
            }
        }
    }
}
