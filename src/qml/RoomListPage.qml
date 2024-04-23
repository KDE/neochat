// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

Kirigami.Page {
    id: root

    /**
     * @brief The current width of the room list.
     *
     * @note Other objects can access the value but the private function makes sure
     *       that only the internal members can modify it.
     */
    readonly property int currentWidth: _private.currentWidth + spaceDrawer.width + 1

    required property NeoChatConnection connection

    readonly property bool collapsed: Config.collapsed

    signal search

    onCurrentWidthChanged: pageStack.defaultColumnWidth = root.currentWidth
    Component.onCompleted: pageStack.defaultColumnWidth = root.currentWidth


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
                RoomManager.resolveResource(item.currentRoom.id)
                return;
            }
        }
    }

    function goToPreviousRoomFiltered(condition) {
        let index = treeView.rowAtIndex(RoomManager.sortFilterRoomTreeModel.currentRoomIndex());
        while (index-- > 0) {
            let item = treeView.itemAtIndex(treeView.index(index, 0))
            if (condition(item)) {
                RoomManager.resolveResource(item.currentRoom.id)
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
        goToNextRoomFiltered(item => (item && item instanceof RoomDelegate && item.hasUnread));
    }

    function goToPreviousUnreadRoom() {
        goToPreviousRoomFiltered(item => (item && item instanceof RoomDelegate && item.hasUnread));
    }

    titleDelegate: Loader {
        Layout.fillWidth: true
        sourceComponent: Kirigami.Settings.isMobile ? userInfo : exploreComponent
    }

    padding: 0

    Connections {
        target: RoomManager
        function onCurrentSpaceChanged() {
            treeView.expandRecursively();
        }

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
                        }
                    }

                    DelegateChoice {
                        roleValue: "addDirect"
                        delegate: Delegates.RoundedItemDelegate {
                            text: i18n("Find your friends")
                            icon.name: "list-add-user"
                            icon.width: Kirigami.Units.gridUnit * 2
                            icon.height: Kirigami.Units.gridUnit * 2

                            onClicked: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
                                connection: root.connection
                            }, {
                                title: i18nc("@title", "Find your friends")
                            })
                        }
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
            return spaceDrawer.showDirectChats ? i18n("No friends found") : i18n("No rooms found");
        } else {
            return spaceDrawer.showDirectChats ? i18n("You haven't added any of your friends yet, click below to search for them.") : i18n("Join some rooms to get started");
        }
        helpfulAction: spaceDrawer.showDirectChats ? userSearchAction : exploreRoomAction

        Kirigami.Action {
            id: exploreRoomAction
            icon.name: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? "search" : "list-add"
            text: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? i18n("Search in room directory") : i18n("Explore rooms")
            onTriggered: {
                let dialog = pageStack.layers.push(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                    connection: root.connection,
                    keyword: RoomManager.sortFilterRoomTreeModel.filterText
                }, {
                    title: i18nc("@title", "Explore Rooms")
                });
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    RoomManager.resolveResource(roomId.length > 0 ? roomId : alias, isJoined ? "" : "join");
                });
            }
        }

        Kirigami.Action {
            id: userSearchAction
            icon.name: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? "search" : "list-add"
            text: RoomManager.sortFilterRoomTreeModel.filterText.length > 0 ? i18n("Search in friend directory") : i18n("Find your friends")
            onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'UserSearchPage'), {
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

    MouseArea {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        parent: applicationWindow().overlay.parent

        x: root.currentWidth - width / 2
        width: Kirigami.Units.smallSpacing * 2
        z: root.z + 1
        enabled: RoomManager.hasOpenRoom && applicationWindow().width >= Kirigami.Units.gridUnit * 35
        visible: enabled
        cursorShape: Qt.SplitHCursor

        property int _lastX

        onPressed: mouse => {
            _lastX = mouse.x;
        }
        onPositionChanged: mouse => {
            if (_lastX == -1) {
                return;
            }
            if (mouse.x > _lastX) {
                // we moved to the right
                if (_private.currentWidth < _private.collapseWidth && _private.currentWidth + (mouse.x - _lastX) >= _private.collapseWidth) {
                    // Here we get back directly to a more wide mode.
                    _private.currentWidth = _private.defaultWidth;
                    Config.collapsed = false;
                } else if (_private.currentWidth >= _private.collapseWidth) {
                    // Increase page width
                    _private.currentWidth = Math.min(_private.defaultWidth, _private.currentWidth + (mouse.x - _lastX));
                }
            } else if (mouse.x < _lastX) {
                const tmpWidth = _private.currentWidth - (_lastX - mouse.x);
                if (tmpWidth < _private.collapseWidth) {
                    _private.currentWidth = Qt.binding(() => _private.collapsedSize);
                    Config.collapsed = true;
                } else {
                    _private.currentWidth = tmpWidth;
                }
            }
        }
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
            desiredWidth: root.width - Kirigami.Units.largeSpacing
            collapsed: root.collapsed
            connection: root.connection

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
        property int currentWidth: Config.collapsed ? collapsedSize : defaultWidth
        readonly property int defaultWidth: Kirigami.Units.gridUnit * 17
        readonly property int collapseWidth: Kirigami.Units.gridUnit * 10
        readonly property int collapsedSize: Kirigami.Units.gridUnit + (Config.compactRoomList ? 0 : Kirigami.Units.largeSpacing * 2) + Kirigami.Units.largeSpacing * 2 + (scrollView.QQC2.ScrollBar.vertical.visible ? scrollView.QQC2.ScrollBar.vertical.width : 0)
    }
}
