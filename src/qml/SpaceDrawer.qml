// SPDX-FileCopyrightText: 2020-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Bart De Vries <bart@mogwai.be>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Control {
    id: root

    readonly property real pinnedWidth: Kirigami.Units.gridUnit * 6
    required property NeoChatConnection connection

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property string selectedSpaceId: RoomManager.lastSpaceId
    Connections {
        target: RoomManager
        function onConnectionChanged() {
            // We need to rebind as any previous change will have been overwritten.
            selectedSpaceId = RoomManager.lastSpaceId;
        }
    }

    property bool showDirectChats: RoomManager.directChatsActive

    signal selectionChanged
    signal spacesUpdated

    contentItem: Loader {
        id: sidebarColumn
        z: 0

        sourceComponent: ColumnLayout {
            spacing: 0

            QQC2.ScrollView {
                id: scrollView
                Layout.fillWidth: true
                Layout.fillHeight: true

                QQC2.ScrollBar.vertical.policy: QQC2.ScrollBar.AlwaysOff
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                contentWidth: -1 // disable horizontal scroll

                background: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                }

                ColumnLayout {
                    id: column
                    width: scrollView.width
                    spacing: 0

                    AvatarTabButton {
                        id: notificationsButton

                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing / 2
                        Layout.bottomMargin: Kirigami.Units.smallSpacing / 2
                        text: i18n("View notifications")
                        contentItem: Kirigami.Icon {
                            source: "notifications"
                        }

                        onClicked: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'NotificationsView.qml'), {
                            connection: root.connection
                        }, {
                            title: i18nc("@title", "Notifications")
                        })
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                    }

                    AvatarTabButton {
                        id: allRoomButton

                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing / 2

                        text: i18n("Home")
                        contentItem: Kirigami.Icon {
                            source: "user-home-symbolic"
                        }

                        checked: root.selectedSpaceId === "" && root.showDirectChats === false
                        onClicked: {
                            root.showDirectChats = false;
                            RoomManager.directChatsActive = false;
                            root.selectedSpaceId = "";
                            RoomManager.lastSpaceId = "";
                            root.selectionChanged();
                        }

                        QQC2.Label {
                            id: homeNotificationCountLabel
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.rightMargin: Kirigami.Units.smallSpacing / 2
                            z: 1
                            width: Math.max(homeNotificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                            height: Kirigami.Units.iconSizes.smallMedium

                            text: root.connection.homeNotifications > 0 ? root.connection.homeNotifications : ""
                            visible: root.connection.homeNotifications > 0 && (root.selectedSpaceId !== "" || root.showDirectChats === true)
                            color: Kirigami.Theme.textColor
                            horizontalAlignment: Text.AlignHCenter
                            background: Rectangle {
                                visible: true
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                Kirigami.Theme.inherit: false
                                color: root.connection.homeHaveHighlightNotifications ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                                radius: height / 2
                            }

                            TextMetrics {
                                id: homeNotificationCountTextMetrics
                                text: homeNotificationCountLabel.text
                            }
                        }
                    }
                    AvatarTabButton {
                        id: directChatButton

                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing / 2

                        text: i18nc("@button View all one-on-one chats with your friends.", "Friends")
                        contentItem: Kirigami.Icon {
                            source: "system-users"
                        }

                        checked: root.showDirectChats === true
                        onClicked: {
                            root.showDirectChats = true;
                            RoomManager.directChatsActive = true;
                            root.selectedSpaceId = "";
                            RoomManager.lastSpaceId = "";
                            root.selectionChanged();
                        }

                        QQC2.Label {
                            id: directChatNotificationCountLabel
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.rightMargin: Kirigami.Units.smallSpacing / 2
                            z: 1
                            width: Math.max(directChatNotificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                            height: Kirigami.Units.iconSizes.smallMedium

                            text: root.connection.directChatNotifications > 0 ? root.connection.directChatNotifications : ""
                            visible: (root.connection.directChatNotifications > 0 || root.connection.directChatInvites) && root.showDirectChats === false
                            color: Kirigami.Theme.textColor
                            horizontalAlignment: Text.AlignHCenter
                            background: Rectangle {
                                visible: true
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                Kirigami.Theme.inherit: false
                                color: root.connection.directChatsHaveHighlightNotifications ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                                radius: height / 2
                            }

                            TextMetrics {
                                id: directChatNotificationCountTextMetrics
                                text: directChatNotificationCountLabel.text
                            }
                        }
                    }

                    Repeater {
                        model: SortFilterSpaceListModel {
                            sourceModel: RoomListModel {
                                connection: root.connection
                            }
                            onLayoutChanged: root.spacesUpdated()
                        }
                        onCountChanged: {
                            if (!root.connection.room(root.selectedSpaceId)) {
                                root.selectedSpaceId = "";
                            }
                        }

                        delegate: AvatarTabButton {
                            id: spaceDelegate

                            required property string displayName
                            required property string avatar
                            required property string roomId
                            required property var currentRoom

                            Layout.fillWidth: true
                            Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                            Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                            text: displayName
                            source: avatar ? ("image://mxc/" + avatar) : ""

                            onSelected: {
                                root.showDirectChats = false;
                                RoomManager.directChatsActive = false;
                                if (!SpaceHierarchyCache.isSpaceChild(roomId, RoomManager.currentRoom.id) || root.selectedSpaceId == roomId) {
                                    RoomManager.resolveResource(currentRoom.id);
                                } else {
                                    RoomManager.lastSpaceId = currentRoom.id;
                                }
                                root.selectedSpaceId = roomId;
                                root.selectionChanged();
                            }
                            checked: root.selectedSpaceId === roomId
                            onContextMenuRequested: root.createContextMenu(currentRoom)

                            QQC2.Label {
                                id: notificationCountLabel
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.rightMargin: Kirigami.Units.smallSpacing / 2
                                z: 1
                                width: Math.max(notificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                                height: Kirigami.Units.iconSizes.smallMedium

                                text: spaceDelegate.currentRoom.childrenNotificationCount > 0 ? spaceDelegate.currentRoom.childrenNotificationCount : ""
                                visible: spaceDelegate.currentRoom.childrenNotificationCount > 0 && root.selectedSpaceId != spaceDelegate.roomId
                                color: Kirigami.Theme.textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                background: Rectangle {
                                    visible: true
                                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                    Kirigami.Theme.inherit: false
                                    color: spaceDelegate.currentRoom.childrenHaveHighlightNotifications ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                                    radius: height / 2
                                }

                                TextMetrics {
                                    id: notificationCountTextMetrics
                                    text: notificationCountLabel.text
                                }
                            }
                        }
                    }

                    AvatarTabButton {
                        id: recommendedSpaceButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                        visible: SpaceHierarchyCache.recommendedSpaceId.length > 0 && !root.connection.room(SpaceHierarchyCache.recommendedSpaceId) && !SpaceHierarchyCache.recommendedSpaceHidden

                        text: i18nc("Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)
                        source: SpaceHierarchyCache.recommendedSpaceAvatar.length > 0 ? root.connection.makeMediaUrl(SpaceHierarchyCache.recommendedSpaceAvatar) : ""
                        onClicked: {
                            recommendedSpaceDialogComponent.createObject(QQC2.ApplicationWindow.overlay, {
                                connection: root.connection
                            }).open();
                        }
                        Component {
                            id: recommendedSpaceDialogComponent
                            RecommendedSpaceDialog {}
                        }
                        Rectangle {
                            color: Kirigami.Theme.backgroundColor
                            width: Kirigami.Units.gridUnit * 1.5
                            height: width
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: Kirigami.Units.smallSpacing
                            anchors.rightMargin: Kirigami.Units.smallSpacing * 2
                            anchors.right: parent.right
                            radius: width / 2
                            z: parent.z + 1
                            Kirigami.Icon {
                                anchors.fill: parent
                                z: parent + 1
                                source: "list-add"
                            }
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Layout.topMargin: Kirigami.Units.smallSpacing / 2
                        Layout.bottomMargin: Kirigami.Units.smallSpacing / 2
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                    }

                    AvatarTabButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                        text: i18n("Create a space")
                        contentItem: Kirigami.Icon {
                            source: "list-add"
                        }
                        onClicked: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'CreateRoomDialog.qml'), {
                            connection: root.connection,
                            isSpace: true,
                            title: i18nc("@title", "Create a Space")
                        }, {
                            title: i18nc("@title", "Create a Space")
                        })
                    }
                }
            }
        }
    }

    function createContextMenu(room) {
        let context = spaceListContextMenu.createObject(root, {
            room: room,
            connection: root.connection
        });
        context.open();
    }
    Component {
        id: spaceListContextMenu
        SpaceListContextMenu {}
    }
}
