import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

AvatarContainer {
    DownloadableContent {
        id: downloadable

        width: downloadButton.width
        height: downloadButton.height

        Button {
            id: downloadButton

            text: content.body

            highlighted: !sentByMe

            onClicked: downloadable.downloadAndOpen()
        }
    }
}
