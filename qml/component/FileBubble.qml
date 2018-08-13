import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

AvatarContainer {
    readonly property var downloadAndOpen: downloadable.downloadAndOpen
    readonly property var saveFileAs: downloadable.saveFileAs

    DownloadableContent {
        id: downloadable

        width: downloadDelegate.width
        height: downloadDelegate.height

        TextDelegate {
            id: downloadDelegate

            maximumWidth: messageListView.width
            highlighted: !sentByMe
            timeLabelVisible: false
            authorLabelVisible: messageRow.avatarVisible

            displayText: "<b>File: </b>" + content.body
        }
    }
}
