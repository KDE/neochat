// SPDX-FileCopyrightText: 2020-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Bart De Vries <bart@mogwai.be>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import '.'
import org.kde.neochat 1.0

QQC2.Control {
    id: root

    readonly property real pinnedWidth: Kirigami.Units.gridUnit * 6
    property bool drawerEnabled: true

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property string selectedSpaceId

    contentItem: Loader {
        id: sidebarColumn
        active: root.drawerEnabled
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

                ColumnLayout {
                    id: column
                    width: scrollView.width
                    spacing: 0

                    AvatarTabButton {
                        id: allRoomButton

                        Layout.fillWidth: true
                        Layout.preferredHeight: width - Kirigami.Units.smallSpacing
                        Layout.maximumHeight: width - Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing

                        text: i18n("All Rooms")
                        source: "globe"

                        contentItem: Kirigami.Icon {
                            source: "globe"
                        }

                        checked: root.selectedSpaceId === ""
                        onClicked: root.selectedSpaceId = ""
                    }

                    Repeater {
                        model: SortFilterSpaceListModel {
                            sourceModel: RoomListModel {
                                connection: Controller.activeConnection
                            }
                        }
                        onCountChanged: {
                            root.enabled = count > 0
                            if (!Controller.activeConnection.room(root.selectedSpaceId)) {
                                root.selectedSpaceId = ""
                            }
                        }
                        Component.onCompleted: root.enabled = count > 0

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

                            onClicked: root.selectedSpaceId = roomId
                            checked: root.selectedSpaceId === roomId
                            onContextMenuRequested: root.createContextMenu(currentRoom)
                        }
                    }
                }
            }
        }
    }

    function createContextMenu(room) {
        let context = spaceListContextMenu.createObject(root, {
            room: room
        });
        context.open()
    }
    Component {
        id: spaceListContextMenu
        SpaceListContextMenu {}
    }
}
