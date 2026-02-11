// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

Kirigami.Page {
    id: root

    Kirigami.ColumnView.interactiveResizeEnabled: true
    Kirigami.ColumnView.minimumWidth: _private.collapsedSize + spaceDrawer.width + 1
    Kirigami.ColumnView.maximumWidth: _private.defaultWidth + spaceDrawer.width + 1
    Kirigami.ColumnView.onInteractiveResizingChanged: {
        if (!Kirigami.ColumnView.interactiveResizing && collapsed) {
            Kirigami.ColumnView.preferredWidth = root.Kirigami.ColumnView.minimumWidth;
        }
    }
    Kirigami.ColumnView.preferredWidth: _private.currentWidth + spaceDrawer.width + 1
    Kirigami.ColumnView.onPreferredWidthChanged: {
        if (width > _private.collapseWidth) {
            NeoChatConfig.collapsed = false;
        } else if (Kirigami.ColumnView.interactiveResizing) {
            NeoChatConfig.collapsed = true;
        }
    }

    required property NeoChatConnection connection

    readonly property bool collapsed: NeoChatConfig.collapsed

    signal search

    onCollapsedChanged: {
        if (collapsed) {
            RoomManager.sortFilterRoomTreeModel.filterText = "";
        }
    }

    function goToNextRoomFiltered(condition) {
        let index = treeView.rowAtIndex(RoomManager.sortFilterRoomTreeModel.currentRoomIndex());
        while (index++ < treeView.rows) {
            let item = treeView.itemAtIndex(treeView.index(index, 0))
            if (condition(item)) {
                RoomManager.resolveResource((item as RoomDelegate).currentRoom.id)
                return;
            }
        }
    }

    function goToPreviousRoomFiltered(condition) {
        let index = treeView.rowAtIndex(RoomManager.sortFilterRoomTreeModel.currentRoomIndex());
        while (index-- > 0) {
            let item = treeView.itemAtIndex(treeView.index(index, 0))
            if (condition(item)) {
                RoomManager.resolveResource((item as RoomDelegate).currentRoom.id)
                return;
            }
        }
    }

    function goToNextRoom() {
        goToNextRoomFiltered(item => (item && item instanceof RoomDelegate));
    }

    function goToPreviousRoom() {
        goToPreviousRoomFiltered(item => (item && item instanceof RoomDelegate));
    }

    function goToNextUnreadRoom() {
        goToNextRoomFiltered(item => (item && item instanceof RoomDelegate && item.hasUnreadMessages));
    }

    function goToPreviousUnreadRoom() {
        goToPreviousRoomFiltered(item => (item && item instanceof RoomDelegate && item.hasUnreadMessages));
    }

    titleDelegate: Loader {
        Layout.fillWidth: true
        sourceComponent: exploreComponent
    }

    padding: 0

    Connections {
        target: RoomManager
        function onCurrentSpaceChanged() {
            treeView.expandRecursively();
        }
    }

    Connections {
        target: RoomManager.sortFilterRoomTreeModel
        function onCurrentRoomChanged() {
            treeView.positionViewAtIndex(RoomManager.sortFilterRoomTreeModel.currentRoomIndex(), TableView.AlignVCenter)
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SpaceDrawer {
            id: spaceDrawer
            Layout.preferredWidth: Kirigami.Units.gridUnit * 3
            Layout.fillHeight: true

            connection: root.connection
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
        }

        QQC2.ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
                Kirigami.Theme.colorSet: Kirigami.Theme.View
            }

            Keys.onDownPressed: ; // Do not delete 🫠
            Keys.onUpPressed: ; // These make sure the scrollview doesn't also scroll while going through the roomlist using the arrow keys

            contentItem: TreeView {
                id: treeView
                topMargin: Math.round(Kirigami.Units.smallSpacing / 2)

                clip: true
                reuseItems: false

                model: RoomManager.sortFilterRoomTreeModel

                selectionModel: ItemSelectionModel {}

                delegate: DelegateChooser {
                    role: "delegateType"

                    DelegateChoice {
                        roleValue: "section"
                        delegate: RoomTreeSection {
                            collapsed: root.collapsed
                        }
                    }

                    DelegateChoice {
                        roleValue: "normal"
                        delegate: RoomDelegate {
                            id: roomDelegate
                            required property int row
                            required property TreeView treeView
                            required property bool current
                            onCurrentChanged: if (current) {
                                forceActiveFocus(Qt.TabFocusReason);
                            }

                            implicitWidth: treeView.width
                            connection: root.connection
                            collapsed: root.collapsed
                            highlighted: RoomManager.currentRoom === currentRoom
                            openOnDrag: true
                        }
                    }

                    DelegateChoice {
                        roleValue: "addDirect"
                        delegate: Delegates.RoundedItemDelegate {
                            text: i18nc("@action:button", "Find your friends")
                            icon.name: "list-add-user"
                            icon.width: Kirigami.Units.gridUnit * 2
                            icon.height: Kirigami.Units.gridUnit * 2

                            onClicked: (Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                                connection: root.connection
                            }, {
                                title: i18nc("@title", "Find your friends")
                            })
                        }
                    }
                }

                // This is a bit silly but it guarantees that the room item heights
                // update promptly on change.
                Connections {
                    target: NeoChatConfig

                    function onCompactRoomListChanged() {
                        treeView.collapseRecursively()
                        treeView.expandRecursively()
                    }
                }
            }
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: (spaceDrawer.width + 1) / 2
        width: scrollView.width - Kirigami.Units.largeSpacing * 4
        visible: treeView.rows == 0
        text: if (RoomManager.sortFilterRoomTreeModel.filterText.length > 0) {
            return RoomManager.currentSpace === "DM" ? i18nc("@info", "No friends found") : i18nc("@info", "No rooms found");
        } else {
            return RoomManager.currentSpace === "DM" ? i18nc("@info", "You haven't added any of your friends yet, click below to search for them.") : i18nc("@info", "Join some rooms to get started");
        }
        helpfulAction: RoomManager.currentSpace === "DM" ? userSearchAction : exploreRoomAction

        Kirigami.Action {
            id: exploreRoomAction
            icon.name: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? "search" : "compass"
            text: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? i18nc("@action:button Search public directory for this room", "Search for Room") : i18nc("@action:button Explore public rooms and spaces", "Explore")
            onTriggered: {
                let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).layers.push(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                    connection: root.connection,
                    keyword: RoomManager.sortFilterRoomTreeModel.filterText
                }, {});
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                });
            }
        }

        Kirigami.Action {
            id: userSearchAction
            icon.name: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? "search" : "list-add-user"
            text: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? i18nc("@action:button", "Search in friend directory") : i18nc("@action:button", "Find your friends")
            onTriggered: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Find your friends")
            })
        }
    }

    footer: Loader {
        width: parent.width
        sourceComponent: Kirigami.Settings.isMobile ? exploreComponentMobile : userInfoDesktop
    }

    Component {
        id: userInfo
        UserInfo {
            bottomEdge: false
            connection: root.connection
        }
    }

    Component {
        id: userInfoDesktop
        UserInfoDesktop {
            connection: root.connection
            collapsed: root.collapsed
        }
    }

    Component {
        id: exploreComponent
        ExploreComponent {
            connection: root.connection
            collapsed: root.collapsed

            onSearch: root.search()

            onTextChanged: newText => {
                RoomManager.sortFilterRoomTreeModel.filterText = newText;
                treeView.expandRecursively();
            }
        }
    }

    Component {
        id: exploreComponentMobile
        ExploreComponentMobile {
            connection: root.connection

            onTextChanged: newText => {
                RoomManager.sortFilterRoomTreeModel.filterText = newText;
            }
        }
    }

    /*
     * Hold the modifiable currentWidth in a private object so that only internal
     * members can modify it.
     */
    QtObject {
        id: _private
        property int currentWidth: NeoChatConfig.collapsed ? collapsedSize : defaultWidth
        readonly property int defaultWidth: Kirigami.Units.gridUnit * 15
        readonly property int collapseWidth: Kirigami.Units.gridUnit * 10
        readonly property int collapsedSize: Kirigami.Units.gridUnit + (NeoChatConfig.compactRoomList ? 0 : Kirigami.Units.largeSpacing * 2) + Kirigami.Units.largeSpacing * 2 + (scrollView.QQC2.ScrollBar.vertical.visible ? scrollView.QQC2.ScrollBar.vertical.width : 0)
    }
}
