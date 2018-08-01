import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

AvatarContainer {
    readonly property bool isNotice: eventType === "notice"

    id: messageRow

    Rectangle {
        id: messageRect

        width: Math.min(Math.max(messageText.implicitWidth, (timeText.visible ? timeText.implicitWidth : 0)) + 24, messageListView.width - (!sentByMe ? 40 + messageRow.spacing : 0))
        height: messageText.implicitHeight + (timeText.visible ? timeText.implicitHeight : 0) + 24

        color: isNotice ? "transparent" : !sentByMe ? Material.accent : background
        border.color: Material.accent
        border.width: isNotice ? 2 : 0

        ColumnLayout {
            id: messageColumn

            anchors.fill: parent
            anchors.margins: 12
            spacing: 0

            Label {
                id: messageText
                Layout.maximumWidth: parent.width
                text: display
                color: isNotice || sentByMe ? Material.foreground : "white"

                wrapMode: Label.Wrap
                linkColor: isNotice || sentByMe ? Material.accent : "white"
                //            textFormat: contentType === "text/html" ? Text.RichText : Text.StyledText
                textFormat: Text.StyledText
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Label {
                id: timeText
                visible: Math.abs(time - aboveTime) > 600000 || index == 0
                Layout.alignment: Qt.AlignRight
                text: Qt.formatTime(time, "hh:mm")
                color: isNotice || sentByMe ? "grey" : "white"
                font.pointSize: 8

//                Component.onCompleted: {
//                    console.log("Difference: " + Math.abs(time - aboveTime))
//                    console.log("Index: " + index)
//                }
            }
        }
    }
}
