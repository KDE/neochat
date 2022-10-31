// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root
    property var connection

    property alias keyword: identifierField.text
    property string server

    title: i18n("Explore Rooms")

    Component.onCompleted: identifierField.forceActiveFocus()

    header: QQC2.Control {
        padding: Kirigami.Units.largeSpacing
        contentItem: RowLayout {
            Kirigami.SearchField {
                id: identifierField
                property bool isRoomAlias: text.match(/#(.+):(.+)/g)
                property var room: isRoomAlias ? connection.roomByAlias(text) : null
                property bool isJoined: room != null

                Layout.fillWidth: true

                placeholderText: i18n("Find a room...")
            }

            QQC2.Button {
                id: joinButton

                visible: identifierField.isRoomAlias

                text: identifierField.isJoined ? i18n("View") : i18n("Join")
                highlighted: true

                onClicked: {
                    if (!identifierField.isJoined) {
                        Controller.joinRoom(identifierField.text);
                        // When joining the room, the room will be opened
                    }
                    applicationWindow().pageStack.layers.pop();
                }
            }

            QQC2.ComboBox {
                id: serverField

                // TODO: in KF6 we should be able to switch to using implicitContentWidthPolicy
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10

                Component.onCompleted: currentIndex = 0

                textRole: "url"
                valueRole: "url"
                model: ServerListModel {
                    id: serverListModel
                }

                delegate: Kirigami.BasicListItem {
                    id: serverItem

                    label: isAddServerDelegate ? i18n("Add New Server") : url
                    subtitle: isHomeServer ? i18n("Home Server") : ""

                    onClicked: if (isAddServerDelegate) {
                        addServerSheet.open()
                    }

                    trailing: QQC2.ToolButton {
                        visible: isAddServerDelegate || isDeletable
                        icon.name: isAddServerDelegate ? "list-add" : "dialog-close"
                        text: i18n("Add new server")
                        Accessible.name: text
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: {
                            if (serverField.currentIndex === index && isDeletable) {
                                serverField.currentIndex = 0
                                server = serverField.currentValue
                                serverField.popup.close()
                            }
                            if (isAddServerDelegate) {
                                addServerSheet.open()
                                serverItem.clicked()
                            } else {
                                serverListModel.removeServerAtIndex(index)
                            }
                        }
                    }
                }

                onActivated: {
                    if (currentIndex !== count - 1) {
                        server = currentValue
                    }
                }

                Kirigami.OverlaySheet {
                    id: addServerSheet

                    parent: applicationWindow().overlay

                    title: i18nc("@title:window", "Add server")

                    onSheetOpenChanged: if (!serverUrlField.isValidServer && !sheetOpen) {
                            serverField.currentIndex = 0
                            server = serverField.currentValue
                        } else if (sheetOpen) {
                            serverUrlField.forceActiveFocus()
                        }

                    contentItem: Kirigami.FormLayout {
                        QQC2.Label {
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 20

                            text: serverUrlField.length > 0 ? (serverUrlField.acceptableInput ? (serverUrlField.isValidServer ? i18n("Valid server entered") : i18n("This server cannot be resolved or has already been added")) : i18n("The entered text is not a valid url")) : i18n("Enter server url e.g. kde.org")
                            color: serverUrlField.length > 0 ? (serverUrlField.acceptableInput ? (serverUrlField.isValidServer ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor) : Kirigami.Theme.negativeTextColor) : Kirigami.Theme.textColor
                        }
                        QQC2.TextField {
                            id: serverUrlField

                            property bool isValidServer: false

                            Kirigami.FormData.label: i18n("Server URL")
                            onTextChanged: {
                                if(acceptableInput) {
                                    serverListModel.checkServer(text)
                                }
                            }

                            validator: RegularExpressionValidator {
                                regularExpression: /^[a-zA-Z0-9-]{1,61}\.([a-zA-Z]{2,}|[a-zA-Z0-9-]{2,}\.[a-zA-Z]{2,3})$/
                            }

                            Connections {
                                target: serverListModel
                                function onServerCheckComplete(url, valid) {
                                    if (url == serverUrlField.text && valid) {
                                        serverUrlField.isValidServer = true
                                    }
                                }
                            }
                        }

                        QQC2.Button {
                            id: okButton

                            text: i18nc("@action:button", "Ok")
                            enabled: serverUrlField.acceptableInput && serverUrlField.isValidServer
                            onClicked: {
                                serverListModel.addServer(serverUrlField.text)
                                serverField.currentIndex = serverField.indexOfValue(serverUrlField.text)
                                // console.log(serverField.delegate.label)
                                server = serverField.currentValue
                                serverUrlField.text = ""
                                addServerSheet.close();
                            }
                        }
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
                    Controller.joinRoom(roomID)
                    justJoined = true;
                } else {
                    RoomManager.enterRoom(connection.room(roomID))
                }
                applicationWindow().pageStack.layers.pop();
            }
            contentItem: RowLayout {
                Kirigami.Avatar {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

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
                        QQC2.Label {
                            visible: isJoined || justJoined
                            text: i18n("Joined")
                            color: Kirigami.Theme.linkColor
                        }
                    }
                    QQC2.Label {
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
                        QQC2.Label {
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
