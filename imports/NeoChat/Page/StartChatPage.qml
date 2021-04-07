// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

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

                onClicked: {
                    connection.requestDirectChat(identifierField.text);
                    applicationWindow().pageStack.layers.pop();
                }
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
                    id: joinChatButton
                    Layout.alignment: Qt.AlignRight
                    visible: directChats && directChats.length > 0

                    icon.name: "document-send"
                    onClicked: {
                        connection.requestDirectChat(userID);
                        applicationWindow().pageStack.layers.pop();
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignRight
                    icon.name: "irc-join-channel"
                    // We wants to make sure an user can't start more than one
                    // chat with someone.
                    visible: !joinChatButton.visible

                    onClicked: {
                        connection.requestDirectChat(userID);
                        applicationWindow().pageStack.layers.pop();
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
