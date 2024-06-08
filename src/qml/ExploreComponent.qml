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

    signal search

    property Kirigami.Action exploreAction: Kirigami.Action {
        text: i18n("Explore rooms")
        icon.name: "compass"
        onTriggered: {
            let dialog = pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Explore Rooms")
            });
            dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
            });
        }
    }
    property Kirigami.Action chatAction: Kirigami.Action {
        text: i18n("Find your friends")
        icon.name: "list-add-user"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
            connection: root.connection
        }, {
            title: i18nc("@title", "Find your friends")
        })
    }
    property Kirigami.Action roomAction: Kirigami.Action {
        text: i18n("Create a Room")
        icon.name: "system-users-symbolic"
        onTriggered: {
            pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'CreateRoomDialog'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Create a Room")
            });
        }
        shortcut: StandardKey.New
    }
    property Kirigami.Action spaceAction: Kirigami.Action {
        text: i18n("Create a Space")
        icon.name: "list-add"
        onTriggered: {
            pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'CreateRoomDialog'), {
                connection: root.connection,
                isSpace: true,
                title: i18nc("@title", "Create a Space")
            }, {
                title: i18nc("@title", "Create a Space")
            });
        }
    }

    property Kirigami.Action scanAction: Kirigami.Action {
        text: i18n("Scan a QR Code")
        icon.name: "view-barcode-qr"
        onTriggered: pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat", "QrScannerPage"), {
            connection: root.connection
        }, {
            title: i18nc("@title", "Scan a QR Code")
        })
    }

    /**
     * @brief Emitted when the text is changed in the search field.
     */
    signal textChanged(string newText)

    Item {
        Layout.preferredWidth: Kirigami.Units.largeSpacing
    }

    Kirigami.Heading {
        Layout.fillWidth: true
        visible: !root.collapsed
        text: i18nc("@title", "Rooms")
    }
    Item {
        Layout.fillWidth: true
        visible: root.collapsed
    }

    QQC2.ToolButton {
        id: searchButton
        display: QQC2.AbstractButton.IconOnly
        onClicked: root.search();
        icon.name: "search"
        text: i18nc("@action", "Search Room")
        Shortcut {
            sequence: "Ctrl+F"
            onActivated: searchButton.clicked()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        id: menuButton
        Accessible.role: Accessible.ButtonMenu
        Accessible.onPressAction: menuButton.action.trigger()
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        action: Kirigami.Action {
            text: i18nc("@action:button", "Show Menu")
            icon.name: "application-menu-symbolic"
            onTriggered: {
                if (Kirigami.isMobile) {
                    const menu = mobileMenu.createObject();
                    menu.open();
                } else {
                    const menu = desktopMenu.createObject(menuButton);
                    menu.closed.connect(menuButton.toggle);
                    menu.open();
                }
            }
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    Component {
        id: desktopMenu
        QQC2.Menu {
            x: mirrored ? parent.width - width : 0
            y: parent ? parent.height : 0

            modal: true
            dim: false

            QQC2.MenuItem {
                Accessible.onPressAction: action.triggered()
                action: exploreAction
            }
            QQC2.MenuItem {
                Accessible.onPressAction: action.triggered()
                action: chatAction
            }
            QQC2.MenuItem {
                Accessible.onPressAction: action.triggered()
                action: roomAction
            }
            QQC2.MenuItem {
                Accessible.onPressAction: action.triggered()
                action: spaceAction
            }
            QQC2.MenuItem {
                Accessible.onPressAction: action.triggered()
                action: scanAction
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
                Delegates.RoundedItemDelegate {
                    action: scanAction
                    onClicked: menuRoot.close()
                    Layout.fillWidth: true
                }
            }
        }
    }
}
