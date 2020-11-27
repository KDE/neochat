/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Effect 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property var connection

    title: i18n("Start a Chat")

    header: Control {
        padding: Kirigami.Units.largeSpacing
        contentItem: RowLayout {
            Kirigami.SearchField {
                id: identifierField

                property bool isUserID: text.match(/@(.+):(.+)/g)

                Layout.fillWidth: true

                placeholderText: i18n("Find a user...")

                onAccepted: userDictListModel.search()
            }

            Button {
                visible: identifierField.isUserID

                text: i18n("Chat")
                highlighted: true

                onClicked: Controller.createDirectChat(connection, identifierField.text)
            }
        }
    }

    ListView {
        id: userDictListView

        clip: true

        spacing: Kirigami.Units.smallSpacing

        model: UserDirectoryListModel {
            id: userDictListModel

            connection: root.connection
            keyword: identifierField.text
        }

        delegate: Kirigami.AbstractListItem {
            width: userDictListView.width
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

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
                    Layout.alignment: Qt.AlignRight
                    visible: directChats != null

                    icon.name: "document-send"
                    onClicked: {
                        roomListForm.joinRoom(connection.room(directChats[0]))
                        root.close()
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignRight
                    icon.name: "irc-join-channel"

                    onClicked: {
                        Controller.createDirectChat(connection, userID)
                        root.close()
                    }
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: userDictListView.count < 1
            text: i18n("No users available")
        }
    }
}
