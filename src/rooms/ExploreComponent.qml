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

    /**
     * @brief Emitted when the text is changed in the search field.
     */
    signal textChanged(string newText)

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
        text: i18nc("@action", "Search Rooms")
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
        text: i18nc("@action:button", "Show Menu")
        icon.name: "application-menu-symbolic"
        onClicked: {
            const item = menu.createObject(menuButton);
            item.closed.connect(menuButton.toggle);
            item.open();
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    Component {
        id: menu
        QQC2.Menu {
            QQC2.MenuItem {
                text: i18n("Find your friends")
                icon.name: "list-add-user"
                onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Find your friends")
                })
            }

            QQC2.MenuItem {
                text: i18n("Create a Room")
                icon.name: "system-users-symbolic"
                onTriggered: {
                    Qt.createComponent('org.kde.neochat', 'CreateRoomDialog').createObject(root, {
                        connection: root.connection
                    }).open();
                }

                Kirigami.Action {
                    shortcut: StandardKey.New
                    onTriggered: parent.trigger()
                }
            }

            QQC2.MenuItem {
                text: i18n("Scan a QR Code")
                icon.name: "view-barcode-qr"
                onTriggered: pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat", "QrScannerPage"), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Scan a QR Code")
                })
            }
        }
    }
}
