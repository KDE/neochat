// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

ColumnLayout {
    id: root
    spacing: 0

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Emitted when the text is changed in the search field.
     */
    signal textChanged(string newText)

    Kirigami.Separator {
        Layout.fillWidth: true
    }
    Kirigami.NavigationTabBar {
        id: exploreTabBar
        Layout.fillWidth: true
        actions: [
            Kirigami.Action {
                id: infoAction
                text: i18n("Search")
                icon.name: "search"
                onTriggered: {
                    if (explorePopup.visible && explorePopupLoader.sourceComponent == search) {
                        explorePopup.close();
                        exploreTabBar.currentIndex = -1;
                    } else if (explorePopup.visible && explorePopupLoader.sourceComponent != search) {
                        explorePopup.close();
                        explorePopup.open();
                    } else {
                        explorePopup.open();
                    }
                    explorePopupLoader.sourceComponent = search;
                }
            },
            Kirigami.Action {
                text: i18n("Explore rooms")
                icon.name: "compass"
                onTriggered: {
                    let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/ExploreRoomsPage.qml", {
                        connection: root.connection
                    }, {
                        title: i18nc("@title", "Explore Rooms")
                    });
                    dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                        RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                    });
                    exploreTabBar.currentIndex = -1;
                }
            },
            Kirigami.Action {
                text: i18n("Find your friends")
                icon.name: "list-add-user"
                onTriggered: {
                    pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/UserSearchPage.qml", {
                        connection: root.connection
                    }, {
                        title: i18nc("@title", "Find your friends")
                    });
                    exploreTabBar.currentIndex = -1;
                }
            },
            Kirigami.Action {
                text: i18n("Create New")
                icon.name: "list-add"
                onTriggered: {
                    if (explorePopup.visible && explorePopupLoader.sourceComponent == create) {
                        explorePopup.close();
                        exploreTabBar.currentIndex = -1;
                    } else if (explorePopup.visible && explorePopupLoader.sourceComponent != create) {
                        explorePopup.close();
                        explorePopup.open();
                    } else {
                        explorePopup.open();
                    }
                    explorePopupLoader.sourceComponent = create;
                }
            }
        ]
    }

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
                    Layout.fillWidth: true
                    action: Kirigami.Action {
                        text: i18n("Create a Room")
                        icon.name: "system-users-symbolic"
                        onTriggered: {
                            pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {
                                connection: root.connection
                            }, {
                                title: i18nc("@title", "Create a Room")
                            });
                            explorePopup.close();
                        }
                        shortcut: StandardKey.New
                    }
                }
                Delegates.RoundedItemDelegate {
                    Layout.fillWidth: true
                    action: Kirigami.Action {
                        text: i18n("Create a Space")
                        icon.name: "list-add"
                        onTriggered: {
                            pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {
                                connection: root.connection,
                                isSpace: true,
                                title: i18nc("@title", "Create a Space")
                            }, {
                                title: i18nc("@title", "Create a Space")
                            });
                            explorePopup.close();
                        }
                    }
                }
            }
        }
    }
}
