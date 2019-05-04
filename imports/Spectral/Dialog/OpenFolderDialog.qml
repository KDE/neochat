import QtQuick 2.12
import Qt.labs.platform 1.1

FolderDialog {
    signal chosen(string path)

    id: root

    title: "Please choose a folder"

    onAccepted: chosen(folder)
}
