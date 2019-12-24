import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Setting 0.1

import Spectral 0.1

Dialog {
    property var controller
    property var connection

    anchors.centerIn: parent
    width: 360
    height: window.height - 100

    id: root

    title: "Start a Chat"

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: identifierField

            placeholderText: "Room Alias/User ID"
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: publicRoomsListView

            spacing: 8

            model: PublicRoomListModel {
                connection: root.connection
            }

            delegate: ColumnLayout {
                width: publicRoomsListView.width

                Label {
                    Layout.fillWidth: true

                    text: name ? name : "No name"
                    color: MPalette.foreground
                    font.pixelSize: 13
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }

                Label {
                    Layout.fillWidth: true

                    visible: text

                    text: topic ? topic.replace(/(\r\n\t|\n|\r\t)/gm," ") : ""
                    color: MPalette.lighter
                    font.pixelSize: 10
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

//    standardButtons: Dialog.Ok | Dialog.Cancel

//    onAccepted: {
//        var identifier = identifierField.text
//        var firstChar = identifier.charAt(0)
//        if (firstChar == "@") {
//            spectralController.createDirectChat(spectralController.connection, identifier)
//        } else if (firstChar == "!" || firstChar == "#") {
//            spectralController.joinRoom(spectralController.connection, identifier)
//        }
//    }

    onClosed: destroy()
}
