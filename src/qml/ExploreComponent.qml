// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

RowLayout {
    id: root

    property var desiredWidth
    property bool collapsed: false
    required property NeoChatConnection connection

    property Kirigami.Action exploreAction: Kirigami.Action {
        text: i18n("Explore rooms")
        icon.name: "compass"
        onTriggered: {
            let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/JoinRoomPage.qml", {connection: root.connection}, {title: i18nc("@title", "Explore Rooms")})
            dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                if (isJoined) {
                    RoomManager.enterRoom(root.connection.room(roomId))
                } else {
                    RoomManager.openResource(roomId.length > 0 ? roomId : alias, "join")
                }
            })
        }
    }
    property Kirigami.Action chatAction: Kirigami.Action {
        text: i18n("Start a Chat")
        icon.name: "list-add-user"
        onTriggered: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/StartChatPage.qml", {connection: root.connection}, {title: i18nc("@title", "Start a Chat")})
    }
    property Kirigami.Action roomAction: Kirigami.Action {
        text: i18n("Create a Room")
        icon.name: "system-users-symbolic"
        onTriggered: {
            pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {connection: root.connection}, {title: i18nc("@title", "Create a Room")})
        }
        shortcut: StandardKey.New
    }
    property Kirigami.Action spaceAction: Kirigami.Action {
        text: i18n("Create a Space")
        icon.name: "list-add"
        onTriggered: {
            pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {connection: root.connection, isSpace: true, title: i18nc("@title", "Create a Space")}, {title: i18nc("@title", "Create a Space")})
        }
    }

    Kirigami.SearchField {
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        Layout.fillWidth: true
        Layout.preferredWidth: root.desiredWidth ? root.desiredWidth - menuButton.width - root.spacing : -1
        visible: !root.collapsed
        onTextChanged: sortFilterRoomListModel.filterText = text
        KeyNavigation.tab: listView
    }

    QQC2.ToolButton {
        id: menuButton
        Accessible.role: Accessible.ButtonMenu
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        action: Kirigami.Action {
            text: i18n("Create rooms and chats")
            icon.name: "irc-join-channel"
            onTriggered: {
                if (Kirigami.isMobile) {
                    const menu = mobileMenu.createObject();
                    menu.open();
                } else {
                    const menu = desktopMenu.createObject(menuButton);
                    menu.closed.connect(menuButton.toggle)
                    menu.open();
                }
            }
        }

        QQC2.ToolTip {
            text: parent.text
        }
    }

    Component {
        id: desktopMenu
        QQC2.Menu {
            x: mirrored ? parent.width - width : 0
            y: parent ? parent.height : 0

            modal: true
            dim: false

            QQC2.MenuItem {
                action: exploreAction
            }
            QQC2.MenuItem {
                action: chatAction
            }
            QQC2.MenuItem {
                action: roomAction
            }
            QQC2.MenuItem {
                action: spaceAction
            }
        }
    }
    Component {
        id: mobileMenu

        Kirigami.OverlayDrawer {
            id: menuRoot

            edge: Qt.BottomEdge
            parent: applicationWindow().overlay

            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

            ColumnLayout {
                width: parent.width
                spacing: 0

                Kirigami.ListSectionHeader {
                    label: i18n("Create rooms and chats")
                }

                Delegates.RoundedItemDelegate {
                    action: exploreAction
                    onClicked: menuRoot.close()
                    Layout.fillWidth: true
                }

                Delegates.RoundedItemDelegate {
                    action: chatAction
                    onClicked: menuRoot.close()
                    Layout.fillWidth: true
                }

                Delegates.RoundedItemDelegate {
                    action: roomAction
                    onClicked: menuRoot.close()
                    Layout.fillWidth: true
                }

                Delegates.RoundedItemDelegate {
                    action: roomAction
                    onClicked: menuRoot.close()
                    Layout.fillWidth: true
                }
            }
        }
    }
}
