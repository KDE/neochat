// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

import './' as RoomList

Kirigami.ScrollablePage {
    id: root

    /**
     * @brief The current width of the room list.
     *
     * @note Other objects can access the value but the private function makes sure
     *       that only the internal members can modify it.
     */
    readonly property int currentWidth: _private.currentWidth

    readonly property RoomListModel roomListModel: RoomListModel {
        connection: Controller.activeConnection
    }

    readonly property bool collapsed: Config.collapsed

    property var enteredRoom: null

    onCollapsedChanged: if (collapsed) {
        sortFilterRoomListModel.filterText = "";
    }

    header: ColumnLayout {
        visible: !root.collapsed
        spacing: 0

        RoomList.SpaceListView {
            roomListModel: root.roomListModel
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Component {
            id: spaceListContextMenu
            SpaceListContextMenu {}
        }
    }

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            itemSelection.setCurrentIndex(roomListModel.index(roomListModel.rowForRoom(RoomManager.currentRoom), 0), ItemSelectionModel.SelectCurrent)
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
        goToNextRoomFiltered((item) => item.visible);
    }

    function goToPreviousRoom() {
        goToPreviousRoomFiltered((item) => item.visible);
    }

    function goToNextUnreadRoom() {
        goToNextRoomFiltered((item) => (item.visible && item.hasUnread));
    }

    function goToPreviousUnreadRoom() {
        goToPreviousRoomFiltered((item) => (item.visible && item.hasUnread));
    }

    titleDelegate: ExploreComponent {
        Layout.fillWidth: true
        desiredWidth: root.width - Kirigami.Units.largeSpacing
        collapsed: root.collapsed
    }

    ListView {
        id: listView

        activeFocusOnTab: true
        clip: AccountRegistry.count > 1

        header: QQC2.ItemDelegate {
            width: visible ? ListView.view.width : 0
            height: visible ? Kirigami.Units.gridUnit * 2 : 0

            visible: root.collapsed

            topPadding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            onClicked: quickView.item.open();

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

        Layout.fillWidth: true

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count == 0
            text: sortFilterRoomListModel.filterText.length > 0 ? i18n("No rooms found") : i18n("Join some rooms to get started")
            helpfulAction: Kirigami.Action {
                icon.name: sortFilterRoomListModel.filterText.length > 0 ? "search" : "list-add"
                text: sortFilterRoomListModel.filterText.length > 0 ? i18n("Search in room directory") : i18n("Explore rooms")
                onTriggered: pageStack.layers.push("qrc:/JoinRoomPage.qml", {
                    connection: Controller.activeConnection,
                    keyword: sortFilterRoomListModel.filterText
                })
            }
        }

        ItemSelectionModel {
            id: itemSelection
            model: root.roomListModel
            onCurrentChanged: listView.currentIndex = sortFilterRoomListModel.mapFromSource(current).row
        }

        model: SortFilterRoomListModel {
            id: sortFilterRoomListModel

            sourceModel: root.roomListModel
            roomSortOrder: Config.mergeRoomList ? SortFilterRoomListModel.LastActivity : SortFilterRoomListModel.Categories
            onLayoutChanged: {
                listView.currentIndex = sortFilterRoomListModel.mapFromSource(itemSelection.currentIndex).row
            }
        }

        section.property: sortFilterRoomListModel.filterText.length === 0 && !Config.mergeRoomList ? "category" : null
        section.delegate: root.collapsed ? foldButton : sectionHeader

        Component {
            id: sectionHeader
            Kirigami.ListSectionHeader {
                height: implicitHeight
                label: roomListModel.categoryName(section)
                action: Kirigami.Action {
                    onTriggered: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
                }
                contentItem.children: QQC2.ToolButton {
                    icon {
                        name: roomListModel.categoryVisible(section) ? "go-up" : "go-down"
                        width: Kirigami.Units.iconSizes.small
                        height: Kirigami.Units.iconSizes.small
                    }

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

            RoomList.CollapsedRoomDelegate {
                filterText: sortFilterRoomListModel.filterText
            }
        }

        Component {
            id: normalModeListComponent

            RoomList.RoomDelegate {
                filterText: sortFilterRoomListModel.filterText
            }
        }
    }

    footer: UserInfo {
        width: parent.width
        visible: !root.collapsed
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
                    _private.currentWidth = _private.collapseWidth;
                    Config.collapsed = false;
                } else if (_private.currentWidth >= _private.collapseWidth) {
                    // Increase page width
                    _private.currentWidth = Math.min(_private.defaultWidth, _private.currentWidth + (mouse.x - _lastX));
                }
            } else if (mouse.x < _lastX) {
                const tmpWidth = _private.currentWidth - (_lastX - mouse.x);
                if (tmpWidth < _private.collapseWidth) {
                    _private.currentWidth = Qt.binding(() => _private.collapsedSize + (root.contentItem.QQC2.ScrollBar.vertical.visible ? root.contentItem.QQC2.ScrollBar.vertical.width : 0));
                    Config.collapsed = true;
                } else {
                    _private.currentWidth = tmpWidth;
                }
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
        readonly property int collapsedSize: Kirigami.Units.gridUnit * 3 - Kirigami.Units.smallSpacing * 3
    }
}
