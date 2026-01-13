// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia

import org.kde.kirigami as Kirigami

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

    MediaDevices {
        id: devices
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
        text: i18nc("@action", "Search Rooms")

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        id: menuButton

        property QQC2.Menu menuItem: null

        function openMenu(): void {
            if (!menuItem || !menuItem.visible) {
                menuItem = menu.createObject(menuButton) as QQC2.Menu;
                menuItem.closed.connect(menuButton.toggle);
                menuItem.open();
            } else {
                menuItem.dismiss()
            }
        }

        Accessible.role: Accessible.ButtonMenu
        display: QQC2.AbstractButton.IconOnly
        down: pressed || menuItem?.visible
        text: i18nc("@action:button", "Show Menu")
        icon.name: "application-menu-symbolic"

        onPressed: openMenu()
        Keys.onReturnPressed: openMenu()
        Keys.onEnterPressed: openMenu()
        Accessible.onPressAction: openMenu()

        QQC2.ToolTip.visible: hovered && !menuItem?.visible
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    Component {
        id: menu
        QQC2.Menu {
            y: menuButton.height

            QQC2.MenuItem {
                text: i18n("Find your friends")
                icon.name: "list-add-user"
                onTriggered: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Find your friends")
                })
            }

            QQC2.MenuItem {
                text: i18n("Create a Room")
                icon.name: "system-users-symbolic"
                onTriggered: {
                    (Qt.createComponent('org.kde.neochat', 'CreateRoomDialog').createObject(root, {
                        connection: root.connection
                    }) as CreateRoomDialog).open();
                }

                Kirigami.Action {
                    shortcut: StandardKey.New
                    onTriggered: parent.trigger()
                }
            }

            QQC2.MenuItem {
                text: i18n("Scan a QR Code")
                icon.name: "view-barcode-qr"
                visible: devices.videoInputs.length > 0
                onTriggered: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent("org.kde.neochat", "QrScannerPage"), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Scan a QR Code")
                })
            }
        }
    }
}
