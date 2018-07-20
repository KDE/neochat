import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    Rectangle {
        id: messageRect

        width: messageImage.implicitWidth + 24
        height: messageImage.implicitHeight + 24

        color: sentByMe ? background : Material.accent

        DownloadableContent {
            id: downloadable

            width: messageImage.width
            height: messageImage.height
            anchors.centerIn: parent

            Image {
                id: messageImage
                z: -4
                source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: content.body

                    onClicked: mouse.button & Qt.LeftButton ? downloadable.downloadAndOpen() : downloadable.saveFileAs()
                }
            }
        }
    }
}
