import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    readonly property var downloadAndOpen: downloadable.downloadAndOpen
    readonly property var saveFileAs: downloadable.saveFileAs

    Rectangle {
        id: messageRect

        width: messageImage.width + 24
        height: messageImage.height + 24

        color: sentByMe ? background : Material.primary

        DownloadableContent {
            id: downloadable

            width: messageImage.width
            height: messageImage.height
            anchors.centerIn: parent

            AutoImage {
                id: messageImage
                z: -4
                sourceSize: 128
                source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                onClicked: downloadAndOpen()
            }
        }
    }
}
