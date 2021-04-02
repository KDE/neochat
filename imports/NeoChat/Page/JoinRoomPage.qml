/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root
    property var connection

    property alias keyword: identifierField.text
    property string server

    title: i18n("Explore Rooms")

    header: Control {
        padding: Kirigami.Units.largeSpacing
        contentItem: RowLayout {
            Kirigami.SearchField {
                property bool isRoomAlias: text.match(/#(.+):(.+)/g)
                property var room: isRoomAlias ? connection.roomByAlias(text) : null
                property bool isJoined: room != null

                Layout.fillWidth: true

                id: identifierField

                placeholderText: i18n("Find a room...")
            }

            Button {
                id: joinButton

                visible: identifierField.isRoomAlias

                text: identifierField.isJoined ? i18n("View") : i18n("Join")
                highlighted: true

                onClicked: {
                    if (!identifierField.isJoined) {
                        roomManager.actionsHandler.joinRoom(identifierField.text);
                        // When joining the room, the room will be opened
                    }
                    applicationWindow().pageStack.layers.pop();
                }
            }

            ComboBox {
                Layout.maximumWidth: 120

                id: serverField

                editable: currentIndex == 1

                model: [i18n("Local"), i18n("Global"), "matrix.org"]

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
    }

    ListView {
        id: publicRoomsListView
        clip: true
        model: PublicRoomListModel {
            id: publicRoomListModel

            connection: root.connection
            server: root.server
            keyword: root.keyword
        }

        onContentYChanged: {
            if(publicRoomListModel.hasMore && contentHeight - contentY < publicRoomsListView.height + 200)
                publicRoomListModel.next();
        }
        delegate: Kirigami.AbstractListItem {
            property bool justJoined: false
            width: publicRoomsListView.width
            onClicked: {
                if (!isJoined) {
                    roomManager.actionsHandler.joinRoom(roomID)
                    justJoined = true;
                } else {
                    roomManager.enterRoom(connection.room(roomID))
                }
                applicationWindow().pageStack.layers.pop();
            }
            contentItem: RowLayout {
                Kirigami.Avatar {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.normal
                    Layout.preferredHeight: Kirigami.Units.iconSizes.normal

                    source: model.avatar ? ("image://mxc/" + model.avatar) : ""
                    name: name
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            level: 4
                            text: name
                            font.bold: true
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                        Label {
                            visible: isJoined || justJoined
                            text: i18n("Joined")
                            color: Kirigami.Theme.linkColor
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        visible: text
                        text: topic ? topic.replace(/(\r\n\t|\n|\r\t)/gm," ") : ""
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Icon {
                            source: "user"
                            color: Kirigami.Theme.disabledTextColor
                            implicitHeight: Kirigami.Units.iconSizes.small
                            implicitWidth: Kirigami.Units.iconSizes.small
                        }
                        Label {
                            text: memberCount + " " + (alias ?? roomID)
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
