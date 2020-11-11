/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Effect 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Dialog {
    property var room

    anchors.centerIn: parent
    width: 360
    height: Math.min(window.height - 100, 640)

    id: root

    title: "Invite a User"

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

                text: "Add"
                highlighted: true

                onClicked: {
                    room.inviteToRoom(identifierField.text)
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

                connection: root.room.connection
                keyword: identifierField.text
            }

            delegate: Control {
                property bool inRoom: room && room.containsUser(userID)

                width: userDictListView.width
                height: 48

                id: delegate

                padding: 8

                contentItem: RowLayout {
                    spacing: 8

                    Kirigami.Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
                        name: name
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

                        visible: inRoom

                        background: RippleEffect {
                            circular: true
                        }
                    }

                    Control {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        visible: !inRoom

                        background: RippleEffect {
                            circular: true

                            onClicked: {
                                room.inviteToRoom(userID)
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
