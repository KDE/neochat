import QtQuick 2.11
import QtQuick.Controls 2.4

Row {
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
}
