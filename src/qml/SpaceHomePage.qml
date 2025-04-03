// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.libneochat as LibNeoChat
import org.kde.neochat.settings

ColumnLayout {
    id: root

    readonly property NeoChatRoom currentRoom: RoomManager.currentRoom

    anchors.fill: parent

    spacing: 0

    QQC2.Control {
        id: headerItem
        Layout.fillWidth: true
        implicitHeight: headerColumn.implicitHeight

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
        }

        ColumnLayout {
            id: headerColumn
            anchors.centerIn: headerItem
            width: sizeHelper.currentWidth
            spacing: Kirigami.Units.largeSpacing

            GroupChatDrawerHeader {
                id: header
                Layout.fillWidth: true
                room: root.currentRoom
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                QQC2.Button {
                    visible: root.currentRoom.canSendState("invite")
                    text: i18nc("@button", "Invite user to space")
                    icon.name: "list-add-user"
                    onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'InviteUserPage'), {
                        room: root.currentRoom
                    }, {
                        title: i18nc("@title", "Invite a User")
                    })
                }
                QQC2.Button {
                    visible: root.currentRoom.canSendState("m.space.child")
                    text: i18nc("@button", "Add new room")
                    icon.name: "list-add"
                    onClicked: _private.createRoom(root.currentRoom.id)
                }
                QQC2.Button {
                    text: i18nc("@action:button", "Leave this space")
                    icon.name: "go-previous"
                    onClicked: RoomManager.leaveRoom(root.currentRoom)
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Button {
                    id: settingsButton

                    display: QQC2.AbstractButton.IconOnly
                    text: i18nc("'Space' is a matrix space", "Space Settings")
                    onClicked: {
                        RoomSettingsView.openRoomSettings(root.currentRoom, RoomSettingsView.Space);
                        drawer.close();
                    }
                    icon.name: 'settings-configure-symbolic'

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
            }
            Kirigami.SearchField {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                onTextChanged: spaceChildSortFilterModel.filterText = text
            }
        }
        LibNeoChat.DelegateSizeHelper {
            id: sizeHelper
            parentItem: root
            startBreakpoint: Kirigami.Units.gridUnit * 46
            endBreakpoint: Kirigami.Units.gridUnit * 66
            startPercentWidth: 100
            endPercentWidth: 85
            maxWidth: Kirigami.Units.gridUnit * 60
        }
    }
    Kirigami.Separator {
        Layout.fillWidth: true
    }
    QQC2.ScrollView {
        id: hierarchyScrollView
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: !spaceChildrenModel.loading

        TreeView {
            id: spaceTree
            columnWidthProvider: function (column) {
                return spaceTree.width;
            }

            clip: true

            model: SpaceChildSortFilterModel {
                id: spaceChildSortFilterModel
                sourceModel: SpaceChildrenModel {
                    id: spaceChildrenModel
                    space: root.currentRoom
                }
            }

            delegate: SpaceHierarchyDelegate {
                onCreateRoom: _private.createRoom(roomId)
            }
        }

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.View
        }
    }
    QQC2.Control {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: spaceChildrenModel.loading

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
        }

        Loader {
            active: spaceChildrenModel.loading
            anchors.centerIn: parent
            sourceComponent: Kirigami.LoadingPlaceholder {}
        }
    }
    QtObject {
        id: _private
        function createRoom(parentId) {
            let dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'CreateRoomDialog'), {
                title: i18nc("@title", "Create a Child"),
                connection: root.currentRoom.connection,
                parentId: parentId,
                showChildType: true,
                showCreateChoice: true
            }, {
                title: i18nc("@title", "Create a Child")
            });
            dialog.addChild.connect((childId, setChildParent, canonical) => {
                // We have to get a room object from the connection as we may not
                // be adding to the top level parent.
                let parent = root.currentRoom.connection.room(parentId);
                if (parent) {
                    parent.addChild(childId, setChildParent, canonical);
                }
            });
            dialog.newChild.connect(childName => {
                spaceChildrenModel.addPendingChild(childName);
            });
        }
    }
}


