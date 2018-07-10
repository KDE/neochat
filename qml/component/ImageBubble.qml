import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Row {
    id: messageRow

    spacing: 6

    ImageStatus {
        id: avatar

        width: height
        height: 40
        round: false
        visible: !sentByMe
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
        id: messageRect

        width: messageImage.implicitWidth + 24
        height: messageImage.implicitHeight + 24

        color: sentByMe ? "lightgrey" : Material.accent

        Image {
            id: messageImage
            anchors.centerIn: parent
            source: "image://mxc/" + content.url

            MouseArea {
                anchors.fill: parent

                hoverEnabled: true
                propagateComposedEvents: true
                ToolTip.visible: containsMouse
                ToolTip.text: content.body
            }
        }
    }
}
