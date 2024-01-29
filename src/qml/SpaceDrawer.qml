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

    property string selectedSpaceId

    property bool showDirectChats: false

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

                        onClicked: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/NotificationsView.qml", {connection: root.connection}, {
                            title: i18nc("@title", "Notifications")
                        });
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

                        text: i18n("All Rooms")
                        contentItem: Kirigami.Icon {
                            source: "globe"
                        }

                        checked: root.selectedSpaceId === "" && root.showDirectChats === false
                        onClicked: {
                            root.showDirectChats = false
                            root.selectedSpaceId = ""
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
                            root.showDirectChats = true
                            root.selectedSpaceId = ""
                        }

                        QQC2.Label {
                            id: notificationCountLabel
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.rightMargin: Kirigami.Units.smallSpacing / 2
                            z: 1
                            width: Math.max(notificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                            height: Kirigami.Units.iconSizes.smallMedium

                            text: root.connection.directChatNotifications > 0 ? root.connection.directChatNotifications : ""
                            visible: root.connection.directChatNotifications > 0 || root.connection.directChatInvites
                            color: Kirigami.Theme.textColor
                            horizontalAlignment: Text.AlignHCenter
                            background: Rectangle {
                                visible: true
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                color: Kirigami.Theme.positiveTextColor
                                radius: height / 2
                            }

                            TextMetrics {
                                id: notificationCountTextMetrics
                                text: notificationCountLabel.text
                            }
                        }
                    }

                    Repeater {
                        model: SortFilterSpaceListModel {
                            sourceModel: RoomListModel {
                                connection: root.connection
                            }
                        }
                        onCountChanged: {
                            if (!root.connection.room(root.selectedSpaceId)) {
                                root.selectedSpaceId = ""
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
                                root.showDirectChats = false
                                root.selectedSpaceId = roomId
                            }
                            checked: root.selectedSpaceId === roomId
                            onContextMenuRequested: root.createContextMenu(currentRoom)
                        }
                    }

                    AvatarTabButton {
                        id: recommendedSpaceButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing

                        visible: SpaceHierarchyCache.recommendedSpaceId.length > 0 && !root.connection.room(SpaceHierarchyCache.recommendedSpaceId) && !SpaceHierarchyCache.recommendedSpaceHidden

                        text: i18nc("Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)
                        source: root.connection.makeMediaUrl(SpaceHierarchyCache.recommendedSpaceAvatar)
                        onClicked: {
                            let dialog = pageStack.pushDialogLayer(Qt.resolvedUrl("qrc:/org/kde/neochat/qml/RecommendedSpaceDialog.qml"), {
                                connection: root.connection
                            }, {
                                title: i18nc("@title Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)
                            });
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
                        onClicked: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {connection: root.connection, isSpace: true, title: i18nc("@title", "Create a Space")}, {title: i18nc("@title", "Create a Space")})

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
        context.open()
    }
    Component {
        id: spaceListContextMenu
        SpaceListContextMenu {}
    }
}
