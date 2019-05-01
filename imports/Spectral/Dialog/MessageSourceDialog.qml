import QtQuick 2.12
import QtQuick.Controls 2.12

Popup {
    property string sourceText

    anchors.centerIn: parent
    width: 480

    id: root

    modal: true
    padding: 16

    closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

    contentItem: ScrollView {
        clip: true

        Label {
            text: sourceText
        }
    }

    onClosed: destroy()
}

