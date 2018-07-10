import QtQuick 2.11
import QtQuick.Controls 2.4

Item {
    width: parent.width
    height: visible ? childrenRect.height : 0

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    onDownloadedChanged: {
        if (downloaded && openOnFinished)
            openSavedFile()
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
