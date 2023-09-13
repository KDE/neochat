// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    required property NeoChatConnection connection

    property alias keyword: identifierField.text
    property string server

    /**
     * @brief Signal emitted when a room is selected.
     *
     * The signal contains all the room's info so that it can be acted
     * upon as required, e.g. joinng or entering the room or adding the room as
     * the child of a space.
     */
    signal roomSelected(string roomId,
                        string displayName,
                        url avatarUrl,
                        string alias,
                        string topic,
                        int memberCount,
                        bool isJoined)

    title: i18n("Explore Rooms")

    Component.onCompleted: identifierField.forceActiveFocus()

    header: QQC2.Control {
        padding: Kirigami.Units.largeSpacing

        background: Rectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false

            color: Kirigami.Theme.backgroundColor
        }

        contentItem: RowLayout {
            Kirigami.SearchField {
                id: identifierField
                property bool isRoomAlias: text.match(/#(.+):(.+)/g)
                property NeoChatRoom room: isRoomAlias ? connection.roomByAlias(text) : null
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

                    onOpened: if (!serverUrlField.isValidServer && !opened) {
                            serverField.currentIndex = 0
                            server = serverField.currentValue
                        } else if (opened) {
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

        Kirigami.Separator {
            z: 999
            anchors {
                left: parent.left
                right: parent.right
                top: parent.bottom
            }
        }
    }

    ListView {
        id: publicRoomsListView
        topMargin: Kirigami.Units.smallSpacing
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
        delegate: ExplorerDelegate {
            onRoomSelected: (roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                root.roomSelected(roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined);
                root.closeDialog();
            }
        }

        footer: RowLayout {
            width: parent.width

            QQC2.ProgressBar {
                visible: publicRoomsListView.model.loading && publicRoomsListView.count !== 0
                indeterminate: true
                padding: Kirigami.Units.largeSpacing * 2
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.topMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: publicRoomsListView.model.loading && publicRoomsListView.count === 0
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: !publicRoomsListView.model.loading && publicRoomsListView.count === 0
            text: i18nc("@info:label", "No rooms found")
        }
    }
}
