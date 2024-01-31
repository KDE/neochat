// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.config
import org.kde.neochat.accounts

Kirigami.Page {
    id: root

    /**
     * @brief The current width of the room list.
     *
     * @note Other objects can access the value but the private function makes sure
     *       that only the internal members can modify it.
     */
    readonly property int currentWidth: _private.currentWidth + spaceListWidth
    readonly property alias spaceListWidth: spaceDrawer.width

    required property NeoChatConnection connection

    readonly property RoomListModel roomListModel: RoomListModel {
        connection: root.connection
    }
    property bool spaceChanging: false

    readonly property bool collapsed: Config.collapsed

    property var enteredRoom: null

    onCollapsedChanged: if (collapsed) {
        sortFilterRoomListModel.filterText = "";
    }

    Component.onCompleted: Runner.roomListModel = root.roomListModel

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            itemSelection.setCurrentIndex(roomListModel.index(roomListModel.rowForRoom(RoomManager.currentRoom), 0), ItemSelectionModel.SelectCurrent);
        }
    }

    function goToNextRoomFiltered(condition) {
        let index = listView.currentIndex;
        while (index++ !== listView.count - 1) {
            if (condition(listView.itemAtIndex(index))) {
                listView.currentIndex = index;
                listView.currentItem.clicked();
                return;
            }
        }
    }

    function goToPreviousRoomFiltered(condition) {
        let index = listView.currentIndex;
        while (index-- !== 0) {
            if (condition(listView.itemAtIndex(index))) {
                listView.currentIndex = index;
                listView.currentItem.clicked();
                return;
            }
        }
    }

    function goToNextRoom() {
        goToNextRoomFiltered(item => item.visible);
    }

    function goToPreviousRoom() {
        goToPreviousRoomFiltered(item => item.visible);
    }

    function goToNextUnreadRoom() {
        goToNextRoomFiltered(item => (item.visible && item.hasUnread));
    }

    function goToPreviousUnreadRoom() {
        goToPreviousRoomFiltered(item => (item.visible && item.hasUnread));
    }

    titleDelegate: Loader {
        Layout.fillWidth: true
        sourceComponent: Kirigami.Settings.isMobile ? userInfo : exploreComponent
    }

    padding: 0

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SpaceDrawer {
            id: spaceDrawer
            Layout.preferredWidth: spaceDrawer.enabled ? Kirigami.Units.gridUnit * 3 : 0
            Layout.fillHeight: true

            connection: root.connection

            onSelectionChanged: root.spaceChanging = true
            onSpacesUpdated: sortFilterRoomListModel.invalidate()
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

            ListView {
                id: listView

                activeFocusOnTab: true
                clip: true

                topMargin: Math.round(Kirigami.Units.smallSpacing / 2)

                header: QQC2.ItemDelegate {
                    width: visible ? ListView.view.width : 0
                    height: visible ? Kirigami.Units.gridUnit * 2 : 0

                    visible: root.collapsed

                    topPadding: Kirigami.Units.largeSpacing
                    leftPadding: Kirigami.Units.largeSpacing
                    rightPadding: Kirigami.Units.largeSpacing
                    bottomPadding: Kirigami.Units.largeSpacing

                    onClicked: quickView.item.open()

                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: Kirigami.Units.iconSizes.smallMedium
                        source: "search"
                    }

                    Kirigami.Separator {
                        width: parent.width
                        anchors.bottom: parent.bottom
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: listView.count == 0
                    text: if (sortFilterRoomListModel.filterText.length > 0) {
                        return spaceDrawer.showDirectChats ? i18n("No friends found") : i18n("No rooms found");
                    } else {
                        return spaceDrawer.showDirectChats ? i18n("You haven't added any of your friends yet, click below to search for them.") : i18n("Join some rooms to get started");
                    }
                    helpfulAction: spaceDrawer.showDirectChats ? userSearchAction : exploreRoomAction

                    Kirigami.Action {
                        id: exploreRoomAction
                        icon.name: sortFilterRoomListModel.filterText.length > 0 ? "search" : "list-add"
                        text: sortFilterRoomListModel.filterText.length > 0 ? i18n("Search in room directory") : i18n("Explore rooms")
                        onTriggered: {
                            let dialog = pageStack.layers.push("qrc:/org/kde/neochat/qml/ExploreRoomsPage.qml", {
                                connection: root.connection,
                                keyword: sortFilterRoomListModel.filterText
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
                        icon.name: sortFilterRoomListModel.filterText.length > 0 ? "search" : "list-add"
                        text: sortFilterRoomListModel.filterText.length > 0 ? i18n("Search in friend directory") : i18n("Find your friends")
                        onTriggered: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/UserSearchPage.qml", {
                            connection: root.connection
                        }, {
                            title: i18nc("@title", "Find your friends")
                        })
                    }
                }

                ItemSelectionModel {
                    id: itemSelection
                    model: root.roomListModel
                    onCurrentChanged: (current, previous) => listView.currentIndex = sortFilterRoomListModel.mapFromSource(current).row
                }

                model: SortFilterRoomListModel {
                    id: sortFilterRoomListModel

                    sourceModel: root.roomListModel
                    roomSortOrder: SortFilterRoomListModel.Categories
                    onLayoutChanged: {
                        layoutTimer.restart();
                        listView.currentIndex = sortFilterRoomListModel.mapFromSource(itemSelection.currentIndex).row;
                    }
                    activeSpaceId: spaceDrawer.selectedSpaceId
                    mode: spaceDrawer.showDirectChats ? SortFilterRoomListModel.DirectChats : SortFilterRoomListModel.Rooms
                }

                // HACK: This is the only way to guarantee the correct choice when
                // there are multiple property changes that invalidate the filter. I.e.
                // in this case activeSpaceId followed by mode.
                Timer {
                    id: layoutTimer
                    interval: 300
                    onTriggered: if ((spaceDrawer.showDirectChats || spaceDrawer.selectedSpaceId.length < 1) && root.spaceChanging) {
                        RoomManager.resolveResource(listView.itemAtIndex(0).currentRoom.id);
                        root.spaceChanging = false;
                    }
                }

                section {
                    property: "category"
                    delegate: root.collapsed ? foldButton : sectionHeader
                }

                Component {
                    id: sectionHeader
                    Kirigami.ListSectionHeader {
                        height: implicitHeight
                        width: listView.width
                        label: roomListModel.categoryName(section)
                        action: Kirigami.Action {
                            onTriggered: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
                        }

                        QQC2.ToolButton {
                            icon {
                                name: roomListModel.categoryVisible(section) ? "go-up" : "go-down"
                                width: Kirigami.Units.iconSizes.small
                                height: Kirigami.Units.iconSizes.small
                            }
                            text: roomListModel.categoryVisible(section) ? i18nc("Collapse <section name>", "Collapse %1", roomListModel.categoryName(section)) : i18nc("Expand <section name", "Expand %1", roomListModel.categoryName(section))
                            display: QQC2.Button.IconOnly

                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                            onClicked: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
                        }
                    }
                }
                Component {
                    id: foldButton
                    Item {
                        width: ListView.view.width
                        height: visible ? width : 0
                        QQC2.ToolButton {
                            id: button
                            anchors.centerIn: parent

                            icon {
                                name: hovered ? (roomListModel.categoryVisible(section) ? "go-up" : "go-down") : roomListModel.categoryIconName(section)
                                width: Kirigami.Units.iconSizes.smallMedium
                                height: Kirigami.Units.iconSizes.smallMedium
                            }

                            onClicked: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))

                            QQC2.ToolTip.text: roomListModel.categoryName(section)
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        }
                    }
                }

                reuseItems: true
                currentIndex: -1 // we don't want any room highlighted by default

                delegate: root.collapsed ? collapsedModeListComponent : normalModeListComponent

                Component {
                    id: collapsedModeListComponent

                    CollapsedRoomDelegate {
                        filterText: sortFilterRoomListModel.filterText
                    }
                }

                Component {
                    id: normalModeListComponent

                    RoomDelegate {
                        filterText: sortFilterRoomListModel.filterText

                        connection: root.connection

                        height: visible ? implicitHeight : 0

                        visible: categoryVisible || filterText.length > 0
                    }
                }

                footer: Delegates.RoundedItemDelegate {
                    visible: listView.view.count > 0 && spaceDrawer.showDirectChats
                    text: i18n("Find your friends")
                    icon.name: "list-add-user"
                    icon.width: Kirigami.Units.gridUnit * 2
                    icon.height: Kirigami.Units.gridUnit * 2

                    onClicked: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/UserSearchPage.qml", {
                        connection: root.connection
                    }, {
                        title: i18nc("@title", "Find your friends")
                    })
                }
            }
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
            visible: !root.collapsed
            bottomEdge: false
            connection: root.connection
        }
    }

    Component {
        id: userInfoDesktop
        UserInfoDesktop {
            visible: !root.collapsed
            connection: root.connection
        }
    }

    Component {
        id: exploreComponent
        ExploreComponent {
            desiredWidth: root.width - Kirigami.Units.largeSpacing
            collapsed: root.collapsed
            connection: root.connection
        }
    }

    Component {
        id: exploreComponentMobile
        ExploreComponentMobile {
            connection: root.connection

            onTextChanged: newText => {
                sortFilterRoomListModel.filterText = newText;
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
        readonly property int collapsedSize: Kirigami.Units.gridUnit * 3 - Kirigami.Units.smallSpacing * 3 + (scrollView.QQC2.ScrollBar.vertical.visible ? scrollView.QQC2.ScrollBar.vertical.width : 0)
    }
}
