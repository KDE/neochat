import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    readonly property bool isNotice: eventType === "notice"

    id: messageRow

    Rectangle {
        id: messageRect

        width: Math.min(messageText.implicitWidth + 24, messageListView.width - (!sentByMe ? 40 + messageRow.spacing : 0))
        height: messageText.implicitHeight + 24

        color: isNotice ? "transparent" : !sentByMe ? Material.accent : background
        border.color: Material.accent
        border.width: isNotice ? 2 : 0

        Label {
            id: messageText
            text: display
            color: isNotice || sentByMe ? Material.foreground : "white"
            anchors.fill: parent
            anchors.margins: 12
            wrapMode: Label.Wrap
            linkColor: isNotice || sentByMe ? Material.accent : "white"
//            textFormat: contentType === "text/html" ? Text.RichText : Text.StyledText
            textFormat: Text.StyledText
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
