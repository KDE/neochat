// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

Kirigami.NavigationTabBar {
    id: root

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Emitted when the text is changed in the search field.
     */
    signal textChanged(string newText)

    Layout.fillWidth: true

    actions: [
        Kirigami.Action {
            id: homeAction

            text: i18nc("@action:button The 'normal' view of NeoChat including the room list", "Home")
            icon.name: "user-home-symbolic"
            checked: true
        },
        Kirigami.Action {
            id: notificationsAction

            text: i18nc("@action:button View all notifications for this account", "Notifications")
            icon.name: "notifications-symbolic"

            onTriggered: {
                (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'NotificationsView'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Notifications"),
                    modality: Qt.NonModal
                })
                homeAction.checked = true; // Reset back to Home
            }
        },
        Kirigami.Action {
            id: accountAction

            text: i18nc("@action:button Open the account menu", "Account")
            icon.name: "im-user-symbolic"

            onTriggered: {
                accountMenu.popup(root.QQC2.Overlay.overlay);
                homeAction.checked = true; // Reset back to Home
            }

            readonly property AccountMenu accountMenu: AccountMenu {
                connection: root.connection
                window: QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow
            }
        }
    ]

    QQC2.Popup {
        id: explorePopup
        parent: root

        y: -height + 1
        width: root.width
        leftPadding: Kirigami.Units.largeSpacing
        rightPadding: Kirigami.Units.largeSpacing
        bottomPadding: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing

        closePolicy: QQC2.Popup.CloseOnEscape

        contentItem: Loader {
            id: explorePopupLoader
            sourceComponent: search

            function switchComponent(newComponent) {
                if (sourceComponent == search) {
                    root.textChanged("");
                }
                sourceComponent = newComponent;
            }
        }

        background: ColumnLayout {
            spacing: 0
            Kirigami.Separator {
                Layout.fillWidth: true
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Kirigami.Theme.backgroundColor
            }
        }

        Component {
            id: search
            Kirigami.SearchField {
                onTextChanged: root.textChanged(text)
            }
        }
        Component {
            id: create
            ColumnLayout {
                spacing: 0
                Delegates.RoundedItemDelegate {
                    id: createRoomButton
                    Layout.fillWidth: true
                    text: i18nc("@action:button", "Create a Room")
                    icon.name: "system-users-symbolic"
                    onClicked: {
                        (Qt.createComponent('org.kde.neochat', 'CreateRoomDialog').createObject(root, {
                            connection: root.connection
                        }) as CreateRoomDialog).open();
                        explorePopup.close();
                    }

                    Kirigami.Action {
                        shortcut: StandardKey.New
                        onTriggered: createRoomButton.clicked()
                    }
                }
                Delegates.RoundedItemDelegate {
                    Layout.fillWidth: true
                    text: i18nc("@action:button", "Create a Space")
                    icon.name: "list-add"
                    onClicked: {
                        (Qt.createComponent('org.kde.neochat', 'CreateSpaceDialog').createObject(root, {
                            connection: root.connection
                        }) as CreateSpaceDialog).open();
                        explorePopup.close();
                    }
                }
            }
        }
    }
}
