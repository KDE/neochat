// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: root

    readonly property NeoChatRoom currentRoom: RoomManager.currentRoom

    padding: 0

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        spacing: 0

        Item {
            id: headerItem
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            implicitHeight: headerColumn.implicitHeight

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
                        onClicked: applicationWindow().pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/InviteUserPage.qml", {room: root.currentRoom}, {title: i18nc("@title", "Invite a User")})
                    }
                    QQC2.Button {
                        visible: root.currentRoom.canSendState("m.space.child")
                        text: i18nc("@button", "Add new child")
                        icon.name: "list-add"
                        onClicked: _private.createRoom(root.currentRoom.id)
                    }
                    QQC2.Button {
                        text: i18nc("@button", "Leave the space")
                        icon.name: "go-previous"
                        onClicked: RoomManager.leaveRoom(root.currentRoom)
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    QQC2.Button {
                        text: i18nc("@button", "Space settings")
                        icon.name: "settings-configure"
                        display: QQC2.AbstractButton.IconOnly
                        onClicked: applicationWindow().pageStack.pushDialogLayer('qrc:/org/kde/neochat/qml/Categories.qml', {room: root.currentRoom, connection: root.currentRoom.connection}, { title: i18n("Room Settings") })

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
            DelegateSizeHelper {
                id: sizeHelper
                startBreakpoint: Kirigami.Units.gridUnit * 46
                endBreakpoint: Kirigami.Units.gridUnit * 66
                startPercentWidth: 100
                endPercentWidth: 85
                maxWidth: Kirigami.Units.gridUnit * 60

                parentWidth: columnLayout.width
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
                columnWidthProvider: function (column) { return spaceTree.width }

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
                    onEnterRoom: _private.enterRoom(roomId)
                }
            }

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
                Kirigami.Theme.colorSet: Kirigami.Theme.View
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: spaceChildrenModel.loading

            Loader {
                active: spaceChildrenModel.loading
                anchors.centerIn: parent
                sourceComponent: Kirigami.LoadingPlaceholder {}
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
    }

    QtObject {
        id: _private
        function createRoom(parentId) {
            let dialog = applicationWindow().pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/CreateRoomDialog.qml", {
                title: i18nc("@title", "Create a Child"),
                connection: root.currentRoom.connection,
                parentId : parentId,
                showChildType: true,
                showCreateChoice: true
            }, {
                title: i18nc("@title", "Create a Child")
            })
            dialog.addChild.connect((childId, setChildParent) => {
                // We have to get a room object from the connection as we may not
                // be adding to the top level parent.
                let parent = root.currentRoom.connection.room(parentId)
                if (parent) {
                    parent.addChild(childId, setChildParent)
                }
            })
            dialog.newChild.connect(childName => {spaceChildrenModel.addPendingChild(childName)})
        }

        function enterRoom(roomId) {
            let room = root.currentRoom.connection.room(roomId)
            if (room) {
                RoomManager.enterRoom(room)
            }
        }
    }
}
