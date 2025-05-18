// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat.libneochat
import org.kde.neochat.settings as Settings

ColumnLayout {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief Request to leave the given room.
     */
    signal requestLeaveRoom(NeoChatRoom room)

    anchors.fill: parent

    spacing: 0

    Component {
        id: roomMenuComponent

        KirigamiComponents.ConvergentContextMenu {
            Kirigami.Action {
                icon.name: "list-add-symbolic"
                text: i18nc("@action:inmenu", "New Room…")
                onTriggered: _private.createRoom(root.room.id)
            }

            Kirigami.Action {
                icon.name: "list-add-symbolic"
                text: i18nc("@action:inmenu", "New Space…")
                onTriggered: _private.createSpace(root.room.id)
            }

            Kirigami.Action {
                icon.name: "search-symbolic"
                text: i18nc("@action:inmenu", "Existing Room…")
                onTriggered: _private.selectExisting(root.room.id)
            }
        }
    }

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
            width: sizeHelper.availableWidth
            spacing: Kirigami.Units.largeSpacing

            GroupChatDrawerHeader {
                id: header
                Layout.fillWidth: true
                room: root.room
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                QQC2.Button {
                    visible: root.room.canSendState("invite")
                    text: i18nc("@button", "Invite user to space")
                    icon.name: "list-add-user"
                    onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.libneochat', 'InviteUserPage'), {
                        room: root.room
                    }, {
                        title: i18nc("@title", "Invite a User")
                    })
                }
                QQC2.Button {
                    id: addNewButton

                    visible: root.room.canSendState("m.space.child")
                    text: i18nc("@button", "Add to Space")
                    icon.name: "list-add"
                    onClicked: {
                        const menu = roomMenuComponent.createObject(addNewButton);
                        menu.popup();
                    }
                }
                QQC2.Button {
                    text: i18nc("@action:button", "Leave this space")
                    icon.name: "go-previous"
                    onClicked: root.requestLeaveRoom(root.room)
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Button {
                    id: settingsButton

                    display: QQC2.AbstractButton.IconOnly
                    text: i18nc("'Space' is a matrix space", "Space Settings")
                    onClicked: {
                        Settings.RoomSettingsView.openRoomSettings(root.room, Settings.RoomSettingsView.Space);
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
        DelegateSizeHelper {
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
                    space: root.room
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
            const dialog = Qt.createComponent('org.kde.neochat.libneochat', 'CreateRoomDialog').createObject(root, {
                connection: root.room.connection,
                parentId: parentId
            });
            dialog.newChild.connect(childName => {
                spaceChildrenModel.addPendingChild(childName);
            });
            dialog.open();
        }

        function createSpace(parentId) {
            const dialog = Qt.createComponent('org.kde.neochat.libneochat', 'CreateSpaceDialog').createObject(root, {
                connection: root.room.connection,
                parentId: parentId,
            });
            dialog.newChild.connect(childName => {
                spaceChildrenModel.addPendingChild(childName);
            });
            dialog.open();
        }

        function selectExisting(parentId) {
            const dialog = Qt.createComponent('org.kde.neochat.spaces', 'SelectExistingRoomDialog').createObject(root, {
                connection: root.room.connection,
                parentId: parentId,
            });
            dialog.addChild.connect((childId, setChildParent, canonical) => {
                // We have to get a room object from the connection as we may not
                // be adding to the top level parent.
                let parent = root.room.connection.room(parentId);
                if (parent) {
                    parent.addChild(childId, setChildParent, canonical);
                }
            });
            dialog.open();
        }
    }
}


