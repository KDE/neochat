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

    anchors.centerIn: parent
    width: 360
    height: Math.min(window.height - 100, 640)

    id: root

    title: "Start a Chat"

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                property bool isUserID: text.match(/@(.+):(.+)/g)

                Layout.fillWidth: true

                id: identifierField

                placeholderText: "Find a user..."

                onAccepted: {
                    userDictListModel.search()
                }
            }

            Button {
                visible: identifierField.isUserID

                text: "Chat"
                highlighted: true

                onClicked: {
                    controller.createDirectChat(connection, identifierField.text)
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: userDictListView

            clip: true

            spacing: 4

            model: UserDirectoryListModel {
                id: userDictListModel

                connection: root.connection
                keyword: identifierField.text
            }

            delegate: Control {
                width: userDictListView.width
                height: 48

                padding: 8

                contentItem: RowLayout {
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

                            text: userID
                            color: MPalette.lighter
                            font.pixelSize: 10
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }

                    Control {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        visible: directChats != null

                        contentItem: MaterialIcon {
                            icon: "\ue89e"
                            color: MPalette.lighter
                            font.pixelSize: 20
                        }

                        background: RippleEffect {
                            circular: true

                            onClicked: {
                                roomListForm.joinRoom(connection.room(directChats[0]))
                                root.close()
                            }
                        }
                    }

                    Control {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        contentItem: MaterialIcon {
                            icon: "\ue7f0"
                            color: MPalette.lighter
                            font.pixelSize: 20
                        }

                        background: RippleEffect {
                            circular: true

                            onClicked: {
                                controller.createDirectChat(connection, userID)
                                root.close()
                            }
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}

            Label {
                anchors.centerIn: parent

                visible: userDictListView.count < 1

                text: "No users available"
                color: MPalette.foreground
            }
        }
    }

    onClosed: destroy()
}
