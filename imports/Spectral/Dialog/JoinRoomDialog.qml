import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

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
    height: Math.min(window.height - 100, 800)

    id: root

    title: "Explore Rooms"

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                property bool isRoomAlias: text.match(/#(.+):(.+)/g)
                property var room: isRoomAlias ? connection.roomByAlias(text) : null
                property bool isJoined: room != null

                Layout.fillWidth: true

                id: identifierField

                placeholderText: "Find a room..."

                onEditingFinished: {
                    keyword = text
                }
            }

            Button {
                id: joinButton

                visible: identifierField.isRoomAlias

                text: identifierField.isJoined ? "View" : "Join"
                highlighted: true
                flat: identifierField.isJoined

                onClicked: {
                    if (identifierField.isJoined) {
                        roomListForm.joinRoom(identifierField.room)
                    } else {
                        controller.joinRoom(connection, identifierField.text)
                    }
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

            delegate: Control {
                width: publicRoomsListView.width
                height: 48

                padding: 8

                contentItem: RowLayout {
                    spacing: 8

                    Kirigami.Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: model.avatarMediaId ? "image://mxc/" + model.avatarMediaId : ""
                        hint: name
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            spacing: 4

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
                                visible: allowGuests

                                text: "GUESTS CAN JOIN"
                                color: MPalette.lighter
                                font.pixelSize: 10
                                padding: 4

                                background: Rectangle {
                                    color: MPalette.banner
                                }
                            }

                            Label {
                                visible: worldReadable

                                text: "WORLD READABLE"
                                color: MPalette.lighter
                                font.pixelSize: 10
                                padding: 4

                                background: Rectangle {
                                    color: MPalette.banner
                                }
                            }
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

                    MaterialIcon {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16

                        icon: "\ue7fc"
                        color: MPalette.lighter
                        font.pixelSize: 16
                    }

                    Label {
                        Layout.preferredWidth: 36

                        text: memberCount
                        color: MPalette.lighter
                        font.pixelSize: 12
                    }

                    Control {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        visible: isJoined

                        contentItem: MaterialIcon {
                            icon: "\ue89e"
                            color: MPalette.lighter
                            font.pixelSize: 20
                        }

                        background: RippleEffect {
                            circular: true

                            onClicked: {
                                roomListForm.joinRoom(connection.room(roomID))
                                root.close()
                            }
                        }
                    }

                    Control {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        visible: !isJoined

                        contentItem: MaterialIcon {
                            icon: "\ue7f0"
                            color: MPalette.lighter
                            font.pixelSize: 20
                        }

                        background: RippleEffect {
                            circular: true

                            onClicked: {
                                controller.joinRoom(connection, roomID)
                                root.close()
                            }
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}

            onContentYChanged: {
                if(publicRoomListModel.hasMore && contentHeight - contentY < publicRoomsListView.height + 200)
                    publicRoomListModel.next();
            }
        }
    }

    onClosed: destroy()
}
