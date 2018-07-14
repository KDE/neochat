import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    DownloadableContent {
        id: downloadable

        width: downloadButton.width
        height: downloadButton.height

        Button {
            id: downloadButton

            text: content.body
            highlighted: !sentByMe
            flat: true

            onClicked: downloadable.downloadAndOpen()
        }
    }
}
