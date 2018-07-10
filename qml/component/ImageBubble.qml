import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

AvatarContainer {
    Rectangle {
        id: messageRect

        width: messageImage.implicitWidth + 24
        height: messageImage.implicitHeight + 24

        color: sentByMe ? "lightgrey" : Material.accent

        DownloadableContent {
            id: downloadable

            width: messageImage.width
            height: messageImage.height
            anchors.centerIn: parent

            Image {
                id: messageImage
                source: "image://mxc/" + content.url

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true
                    propagateComposedEvents: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: content.body

                    onClicked: downloadable.downloadAndOpen()
                }
            }
        }
    }
}
