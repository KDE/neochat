// SPDX-FileCopyrightText: 2020-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Bart De Vries <bart@mogwai.be>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

QQC2.Control {
    id: root

    readonly property real pinnedWidth: Kirigami.Units.gridUnit * 6
    required property NeoChatConnection connection

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    onActiveFocusChanged: if (activeFocus) {
        notificationsButton.forceActiveFocus();
    }

    contentItem: ColumnLayout {
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
                    visible: !Kirigami.Settings.isMobile // Shows up in the mobile bar instead

                    activeFocusOnTab: true

                    onSelected: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'NotificationsView'), {
                        connection: root.connection
                    }, {
                        title: i18nc("@title", "Notifications"),
                        modality: Qt.NonModal
                    })
                }

                Kirigami.Separator {
                    visible: notificationsButton.visible

                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                }

                AvatarTabButton {
                    id: allRoomButton

                    readonly property int countedNotifications: root.connection.homeNotifications + root.connection.roomInvites
                    readonly property bool hasCountableNotifications: countedNotifications > 0

                    Layout.fillWidth: true
                    Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                    Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                    Layout.topMargin: Kirigami.Units.smallSpacing / 2

                    text: hasCountableNotifications ? i18ncp("Home space for the uncategorized rooms", "Home (%1 notification)", "Home (%1 notifications)", countedNotifications) : i18nc("Home space for the uncategorized rooms", "Home")
                    contentItem: Kirigami.Icon {
                        source: "user-home-symbolic"

                        QQC2.Label {
                            id: homeNotificationCountLabel
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.topMargin: -Kirigami.Units.smallSpacing
                            anchors.rightMargin: -Kirigami.Units.smallSpacing
                            z: 1
                            width: Math.max(homeNotificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                            height: Kirigami.Units.iconSizes.smallMedium

                            text: allRoomButton.countedNotifications
                            visible: allRoomButton.hasCountableNotifications && (RoomManager.currentSpace.length > 0 || RoomManager.currentSpace !== "DM")
                            color: Kirigami.Theme.textColor
                            horizontalAlignment: Text.AlignHCenter
                            background: Rectangle {
                                visible: true
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                Kirigami.Theme.inherit: false
                                color: root.connection.homeHaveHighlightNotifications || root.connection.roomInvites > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                                radius: height / 2
                            }

                            TextMetrics {
                                id: homeNotificationCountTextMetrics
                                text: homeNotificationCountLabel.text
                            }
                        }
                    }

                    activeFocusOnTab: true

                    checked: RoomManager.currentSpace.length === 0
                    onSelected: RoomManager.currentSpace = ""
                }
                AvatarTabButton {
                    id: directChatButton

                    readonly property int countedNotifications: root.connection.directChatNotifications + root.connection.directChatInvites
                    readonly property bool hasCountableNotifications: countedNotifications > 0

                    Layout.fillWidth: true
                    Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                    Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                    Layout.topMargin: Kirigami.Units.smallSpacing / 2

                    text: {
                        if (directChatButton.hasCountableNotifications) {
                            return i18ncp("@button View all one-on-one chats.", "Direct Messages (%1 notification)", "Direct Messages (%1 notifications)", directChatButton.countedNotifications);
                        }

                        return i18nc("@button View all one-on-one chats.", "Direct Messages");
                    }
                    contentItem: Kirigami.Icon {
                        source: "system-users-symbolic"

                        QQC2.Label {
                            id: directChatNotificationCountLabel
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.topMargin: -Kirigami.Units.smallSpacing
                            anchors.rightMargin: -Kirigami.Units.smallSpacing
                            z: 1
                            width: Math.max(directChatNotificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                            height: Kirigami.Units.iconSizes.smallMedium

                            text: visible ? directChatButton.countedNotifications : ""
                            visible: directChatButton.hasCountableNotifications && RoomManager.currentSpace !== "DM"
                            color: Kirigami.Theme.textColor
                            horizontalAlignment: Text.AlignHCenter
                            background: Rectangle {
                                visible: true
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                Kirigami.Theme.inherit: false
                                color: root.connection.directChatsHaveHighlightNotifications || root.connection.directChatInvites > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                                radius: height / 2
                            }

                            TextMetrics {
                                id: directChatNotificationCountTextMetrics
                                text: directChatNotificationCountLabel.text
                            }
                        }
                    }

                    activeFocusOnTab: true

                    checked: RoomManager.currentSpace === "DM"
                    onSelected: RoomManager.currentSpace = "DM"
                }

                Repeater {
                    model: RoomManager.sortFilterSpaceListModel

                    delegate: AvatarTabButton {
                        id: spaceDelegate

                        required property string displayName
                        required property url avatar
                        required property string roomId
                        required property var currentRoom

                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                        text: displayName
                        source: avatar

                        notificationCount: spaceDelegate.currentRoom.childrenNotificationCount
                        notificationHighlight: spaceDelegate.currentRoom.childrenHaveHighlightNotifications
                        showNotificationLabel: spaceDelegate.currentRoom.childrenNotificationCount > 0 && RoomManager.currentSpace != spaceDelegate.roomId

                        activeFocusOnTab: true

                        onSelected: {
                            RoomManager.currentSpace = spaceDelegate.roomId;
                        }
                        checked: RoomManager.currentSpace === roomId
                        onContextMenuRequested: root.createContextMenu(currentRoom)
                    }
                }

                AvatarTabButton {
                    id: recommendedSpaceButton
                    Layout.fillWidth: true
                    Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                    Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                    activeFocusOnTab: true

                    visible: SpaceHierarchyCache.recommendedSpaceId.length > 0 && !root.connection.room(SpaceHierarchyCache.recommendedSpaceId) && !SpaceHierarchyCache.recommendedSpaceHidden

                    text: i18nc("Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)
                    source: SpaceHierarchyCache.recommendedSpaceAvatar.toString().length > 0 ? root.connection.makeMediaUrl(SpaceHierarchyCache.recommendedSpaceAvatar) : ""
                    onSelected: {
                        (recommendedSpaceDialogComponent.createObject(QQC2.Overlay.overlay, {
                            connection: root.connection
                        }) as RecommendedSpaceDialog).open();
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
                    id: createNewButton

                    Layout.fillWidth: true
                    Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                    Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                    text: i18nc("@action:button Create a new room or space", "Create")
                    contentItem: Kirigami.Icon {
                        source: "list-add"
                    }

                    activeFocusOnTab: true
                    down: menu.opened

                    onSelected: menu.popup(root.QQC2.Overlay.overlay, createNewButton.mapToGlobal(Qt.point(createNewButton.width, 0)))

                    KirigamiComponents.ConvergentContextMenu {
                        id: menu

                        Kirigami.Action {
                            text: i18nc("@action:button Create a new room", "New Room…")
                            icon.name: "list-add-symbolic"

                            onTriggered: {
                                (Qt.createComponent('org.kde.neochat', 'CreateRoomDialog').createObject(root, {
                                    connection: root.connection
                                }) as CreateRoomDialog).open();
                            }
                        }

                        Kirigami.Action {
                            text: i18nc("@action:button Create a new room", "New Space…")
                            icon.name: "list-add-symbolic"

                            onTriggered: {
                                (Qt.createComponent('org.kde.neochat', 'CreateSpaceDialog').createObject(root, {
                                    connection: root.connection
                                }) as CreateSpaceDialog).open();
                            }
                        }
                    }
                }

                AvatarTabButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                    Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                    text: i18nc("@action:button Explore public rooms and spaces", "Explore")
                    contentItem: Kirigami.Icon {
                        source: "compass"
                    }

                    activeFocusOnTab: true

                    onSelected: {
                        let dialog = (Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                            connection: root.connection,
                            keyword: RoomManager.sortFilterRoomTreeModel.filterText
                        }, {});
                        dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                            RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                        });
                    }
                }
            }
        }
    }

    function createContextMenu(room: NeoChatRoom): void {
        let context = spaceListContextMenu.createObject(root, {
            room: room,
            connection: root.connection
        }) as SpaceListContextMenu;
        context.popup();
    }
    Component {
        id: spaceListContextMenu
        SpaceListContextMenu {
            window: root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow
        }
    }
}
