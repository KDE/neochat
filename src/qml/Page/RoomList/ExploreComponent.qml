// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.ToolBar {
    id: root

    property var desiredWidth
    property bool collapsed: false

    property Kirigami.Action exploreAction: Kirigami.Action {
        text: i18n("Explore rooms")
        icon.name: "compass"
        onTriggered: {
            applicationWindow().pushReplaceLayer("qrc:/JoinRoomPage.qml", {connection: Controller.activeConnection})
        }
    }
    property Kirigami.Action chatAction: Kirigami.Action {
        text: i18n("Start a Chat")
        icon.name: "list-add-user"
        onTriggered: applicationWindow().pushReplaceLayer("qrc:/StartChatPage.qml", {connection: Controller.activeConnection})
    }
    property Kirigami.Action roomAction: Kirigami.Action {
        text: i18n("Create a Room")
        icon.name: "system-users"
        onTriggered: {
            let dialog = createRoomDialog.createObject(root.overlay);
            dialog.open();
        }
        shortcut: StandardKey.New
    }
    property Kirigami.Action spaceAction: Kirigami.Action {
        text: i18n("Create a Space")
        icon.name: "list-add"
        onTriggered: {
            let dialog = createSpaceDialog.createObject(root.overlay);
            dialog.open()
        }
    }

    padding: 0

    RowLayout {
        id: row
        Kirigami.SearchField {
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            Layout.preferredWidth: root.desiredWidth ? root.desiredWidth - menuButton.width - row.spacing : -1
            visible: !root.collapsed
            onTextChanged: sortFilterRoomListModel.filterText = text
            KeyNavigation.tab: listView
        }
        QQC2.ToolButton {
            id: menuButton
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            action: Kirigami.Action {
                text: i18n("Create rooms and chats")
                icon.name: "irc-join-channel"
                onTriggered: {
                    if (Kirigami.isMobile) {
                        let menu = mobileMenu.createObject();
                        menu.open();
                    } else {
                        let menu = desktopMenu.createObject(menuButton, {y: menuButton.height});
                        menu.closed.connect(menuButton.toggle)
                        menu.open();
                    }
                }
            }
            QQC2.ToolTip {
                text: parent.text
            }
        }
    }

    Component {
        id: desktopMenu
        QQC2.Menu {
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
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            topPadding: 0

            parent: applicationWindow().overlay

            ColumnLayout {
                width: parent.width
                spacing: 0

                Kirigami.ListSectionHeader {
                    label: i18n("Create rooms and chats")
                }
                Kirigami.BasicListItem {
                    implicitHeight: Kirigami.Units.gridUnit * 3
                    action: exploreAction
                    onClicked: menuRoot.close()
                }
                Kirigami.BasicListItem {
                    implicitHeight: Kirigami.Units.gridUnit * 3
                    action: chatAction
                    onClicked: menuRoot.close()
                }
                Kirigami.BasicListItem {
                    implicitHeight: Kirigami.Units.gridUnit * 3
                    action: roomAction
                    onClicked: menuRoot.close()
                }
                Kirigami.BasicListItem {
                    implicitHeight: Kirigami.Units.gridUnit * 3
                    action: roomAction
                    onClicked: menuRoot.close()
                }
            }
        }
    }
}
