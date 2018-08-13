import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2

FileDialog {
    id: locationDialog
    title: "Please choose a location"
    folder: shortcuts.home
    selectFolder: true

    onAccepted: currentRoom.downloadFile(eventId, folder + "/" + currentRoom.fileNameToDownload(eventId))
    onVisibleChanged: visible ? {} : locationDialog.destroy()
}
