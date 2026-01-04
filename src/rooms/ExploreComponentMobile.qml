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
            id: infoAction
            text: i18n("Search")
            icon.name: "search"
            onTriggered: {
                if (explorePopup.visible && explorePopupLoader.sourceComponent == search) {
                    explorePopup.close();
                    root.currentIndex = -1;
                } else if (explorePopup.visible && explorePopupLoader.sourceComponent != search) {
                    explorePopup.close();
                    explorePopup.open();
                } else {
                    explorePopup.open();
                }
                explorePopupLoader.switchComponent(search);
            }
        },
        Kirigami.Action {
            text: i18nc("@action:inmenu Explore public rooms and spaces", "Explore")
            icon.name: "compass"
            onTriggered: {
                explorePopup.close();
                let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                    connection: root.connection
                }, {});
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                });
                root.currentIndex = -1;
            }
        },
        Kirigami.Action {
            text: i18n("Find your friends")
            icon.name: "list-add-user"
            onTriggered: {
                explorePopup.close();
                (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Find your friends")
                });
                root.currentIndex = -1;
            }
        },
        Kirigami.Action {
            text: i18n("Create New")
            icon.name: "list-add"
            onTriggered: {
                if (explorePopup.visible && explorePopupLoader.sourceComponent == create) {
                    explorePopup.close();
                    root.currentIndex = -1;
                } else if (explorePopup.visible && explorePopupLoader.sourceComponent != create) {
                    explorePopup.close();
                    explorePopup.open();
                } else {
                    explorePopup.open();
                }
                explorePopupLoader.switchComponent(create);
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
