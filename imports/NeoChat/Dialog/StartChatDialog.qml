/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.0 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Effect 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Dialog {
    property var connection

    anchors.centerIn: parent
    width: 360
    height: Math.min(window.height - 100, 640)

    id: root

    title: i18n("Start a Chat")

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true

            AutoTextField {
                property bool isUserID: text.match(/@(.+):(.+)/g)

                Layout.fillWidth: true

                id: identifierField

                placeholderText: i18n("Find a user...")

                onAccepted: {
                    userDictListModel.search()
                }
            }

            Button {
                visible: identifierField.isUserID

                text: i18n("Chat")
                highlighted: true

                onClicked: {
                    Controller.createDirectChat(connection, identifierField.text)
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

                    Kirigami.Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: avatar
                        name: name
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        spacing: 0

                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: name
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: userID
                            color: Kirigami.Theme.disabledColor
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }

                    Button {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        visible: directChats != null

                        icon.name: "document-send"
                        onClicked: {
                            roomListForm.joinRoom(connection.room(directChats[0]))
                            root.close()
                        }
                    }

                    Button {
                        icon.name: "irc-join-channel"

                        onClicked: {
                            Controller.createDirectChat(connection, userID)
                            root.close()
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}

            Label {
                anchors.centerIn: parent

                visible: userDictListView.count < 1

                text: i18n("No users available")
            }
        }
    }

    onClosed: destroy()
}
