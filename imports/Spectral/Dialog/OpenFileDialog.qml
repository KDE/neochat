import QtQuick 2.12
import QtQuick.Dialogs 1.2

FileDialog {
    signal chosen(string path)

    id: root

    title: "Please choose a file"
    selectMultiple: false

    onAccepted: chosen(selectFolder ? folder : fileUrl)
}
