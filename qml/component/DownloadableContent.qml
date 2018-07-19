import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Dialogs 1.2

Item {
    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    Rectangle {
        z: 5
        height: parent.height
        width: progressInfo.active && !progressInfo.completed ? progressInfo.progress / progressInfo.total * parent.width : 0
        color: Material.accent
        opacity: 0.4
    }

    FileDialog {
        id: locationDialog
        title: "Please choose a location"
        folder: shortcuts.home
        selectFolder: true

        onAccepted: currentRoom.downloadFile(eventId, folder + "/" + currentRoom.fileNameToDownload(eventId))

    }

    onDownloadedChanged: {
        if (downloaded && openOnFinished) openSavedFile()
    }

    function saveFileAs() {
        locationDialog.open()
    }

    function downloadAndOpen()
    {
        if (downloaded)
            openSavedFile()
        else
        {
            openOnFinished = true
            currentRoom.downloadFile(eventId)
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath))
            return;

        if (Qt.openUrlExternally(progressInfo.localDir))
            return;
    }
}
