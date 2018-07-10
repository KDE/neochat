import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Row {
    readonly property bool isNotice: eventType === "notice"

    id: messageRow

    spacing: 6

    ImageStatus {
        id: avatar

        width: height
        height: 40
        round: false
        visible: !sentByMe && aboveAuthor !== author
        source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : null
        displayText: author.displayName

        MouseArea {
            anchors.fill: parent

            hoverEnabled: true
            ToolTip.visible: containsMouse
            ToolTip.text: author.displayName
        }
    }

    Rectangle {
        width: height
        height: 40
        color: "transparent"
        visible: !sentByMe && aboveAuthor === author
    }

    Rectangle {
        id: messageRect

        width: Math.min(messageText.implicitWidth + 24, messageListView.width - (!sentByMe ? avatar.width + messageRow.spacing : 0))
        height: messageText.implicitHeight + 24

        color: isNotice ? "transparent" : sentByMe ? "lightgrey" : Material.accent
        border.color: Material.accent
        border.width: isNotice ? 2 : 0

        Label {
            id: messageText
            text: display
            color: isNotice ? "black" : sentByMe ? "black" : "white"
            anchors.fill: parent
            anchors.margins: 12
            wrapMode: Label.Wrap
            linkColor: isNotice ? Material.accent : sentByMe ? Material.accent : "white"
            textFormat: Text.StyledText
        }
    }
}
