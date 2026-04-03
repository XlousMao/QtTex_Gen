import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

pragma ComponentBehavior: Bound

ApplicationWindow  {
    visible:true
    width: 800
    height: 600
    title: qsTr("Anki Math Card Converter (QtTex_Gen)")

    // 1. 定义打开文件对话框
    FileDialog {
        id: fileDialog
        title: "Please choose a Markdown file"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Markdown files (*.md)", "All files (*)"]
        onAccepted: {
            // 将选中的文件路径 (currentFile) 传递给 C++
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

    //顶部：核心按钮功能
    header: ToolBar{
        RowLayout{
            anchors.fill: parent

            Button { text: "Open File"; onClicked: fileDialog.open()}     

            // 用一个弹性空白把中间的按钮推到中间
            Item { Layout.fillWidth: true }

            //醒目的蓝色转换按钮
            Button {
                text:"Convert LaTeX(to Anki)"
                onClicked:appController.processCards()// 调用 C++ 逻辑
                background: Rectangle {
                    color: "royalblue"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // 再用一个弹性空白把右边的按钮推到右边
            Item { Layout.fillWidth: true }

            Button { text: "Save Result"; onClicked: saveDialog.open()} 
        }
    }

    //主体：分栏布局 (修复拼写错误 SplitView)
    SplitView{
        anchors.fill:parent

        ScrollView{
            SplitView.preferredWidth:parent.width/2
            TextArea{
                id:leftTextArea
                placeholderText:"左侧显示打开的Markdown内容..."
                wrapMode: TextArea.Wrap // 增加自动换行，避免横向滚动条太长
            }
        }
        ScrollView{
            TextArea{
                id:rightTextArea
                placeholderText:"右侧显示转换后的Anki格式内容..."
                wrapMode: TextArea.Wrap
            }
        }
    }


    //底部：状态栏
    footer: Label {
        id: statusLabel
        text: "Ready | 0 Cards Detected"
        padding: 5
    }

     // ===================================
    // 连接 C++ 信号与 QML
    Connections {
        target: appController
        function onFileLoaded(rawContent) { leftTextArea.text = rawContent; }
        function onProcessCompleted(resultText) { rightTextArea.text = resultText; }
        function onErrorMessage(msg) { statusLabel.text = msg; }
    }
}
