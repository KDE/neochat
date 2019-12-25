import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

import Spectral 0.1

Dialog {
    property var controller
    property var connection

    property string keyword
    property string server

    anchors.centerIn: parent
    width: 480
    height: window.height - 100

    id: root

    title: "Start a Chat"

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                Layout.fillWidth: true

                id: identifierField

                placeholderText: "Room Alias/User ID"

                Keys.onReturnPressed: {
                    keyword = text
                }
            }

            ComboBox {
                Layout.maximumWidth: 120

                id: serverField

                editable: currentIndex == 1

                model: ["Local", "Global", "matrix.org"]

                onCurrentIndexChanged: {
                    if (currentIndex == 0) {
                        server = ""
                    } else if (currentIndex == 2) {
                        server = "matrix.org"
                    }
                }

                Keys.onReturnPressed: {
                    if (currentIndex == 1) {
                        server = editText
                    }
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: publicRoomsListView

            clip: true

            spacing: 4

            model: PublicRoomListModel {
                id: publicRoomListModel

                connection: root.connection
                server: root.server
                keyword: root.keyword
            }

            delegate: Item {
                width: publicRoomsListView.width
                height: 40

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4

                    spacing: 8

                    Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: avatar
                        hint: name
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        spacing: 0

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: name
                            color: MPalette.foreground
                            font.pixelSize: 13
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            visible: text

                            text: topic ? topic.replace(/(\r\n\t|\n|\r\t)/gm," ") : ""
                            color: MPalette.lighter
                            font.pixelSize: 10
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }
                }

                RippleEffect {
                    anchors.fill: parent
                }
            }

            ScrollBar.vertical: ScrollBar {}

            onContentYChanged: {
                if(publicRoomListModel.hasMore && contentHeight - contentY < publicRoomsListView.height + 200)
                    publicRoomListModel.next();
            }
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
