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

Kirigami.ScrollablePage {
    id: page

    /**
     * @brief The current width of the room list.
     *
     * @note Other objects can access the value but the private function makes sure
     *       that only the internal members can modify it.
     */
    readonly property int currentWidth: _private.currentWidth

    readonly property bool collapsed: Config.collapsed
    onCollapsedChanged: if (collapsed) {
        sortFilterRoomListModel.filterText = "";
    }

    header: ColumnLayout {
        visible: !page.collapsed
        spacing: 0

        ListView {
            id: spaceList
            property string activeSpaceId: ""

            orientation: Qt.Horizontal
            spacing: Kirigami.Units.smallSpacing
            clip: true
            visible: spaceList.count > 0

            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            Layout.fillWidth: true

            model: SortFilterSpaceListModel {
                id: sortFilterSpaceListModel
                sourceModel: roomListModel
            }

            header: QQC2.ItemDelegate {
                id: homeButton
                icon.name: "home"
                text: i18nc("@action:button", "Show All Rooms")
                height: parent.height
                width: height
                leftPadding: topPadding
                rightPadding: topPadding

                contentItem: Kirigami.Icon {
                    source: "home"
                }

                onClicked: {
                    sortFilterRoomListModel.activeSpaceId = "";
                    spaceList.activeSpaceId = '';
                    listView.positionViewAtIndex(0, ListView.Beginning);
                }

                QQC2.ToolTip.text: homeButton.text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            delegate: QQC2.ItemDelegate {
                required property string avatar
                required property var currentRoom
                required property int index
                required property string id

                height: parent.height
                width: height
                leftPadding: topPadding
                rightPadding: topPadding

                contentItem: Kirigami.Avatar {
                    name: currentRoom.displayName
                    source: avatar !== "" ? "image://mxc/" + avatar : ""
                }

                onClicked: {
                    spaceList.activeSpaceId = id;
                    sortFilterRoomListModel.activeSpaceId = id;
                }

                Accessible.name: currentRoom.displayName

                QQC2.ToolTip.text: currentRoom.displayName
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                onPressAndHold: {
                    spaceList.createContextMenu(currentRoom)
                }
                TapHandler {
                    acceptedButtons: Qt.RightButton
                    acceptedDevices: PointerDevice.Mouse
                    onTapped: spaceList.createContextMenu(currentRoom)
                }
            }
            function createContextMenu(room) {
                const menu = spaceListContextMenu.createObject(page, {room: room})
                menu.open()
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Component {
            id: spaceListContextMenu
            SpaceListContextMenu {}
        }
    }

    property var enteredRoom

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            itemSelection.setCurrentIndex(roomListModel.index(roomListModel.indexForRoom(RoomManager.currentRoom), 0), ItemSelectionModel.SelectCurrent)
        }
    }

    function goToNextRoomFiltered(condition) {
        let index = listView.currentIndex;
        while (index++ !== listView.count - 1) {
            if (condition(listView.itemAtIndex(index))) {
                listView.currentIndex = index;
                listView.currentItem.action.trigger();
                return;
            }
        }
    }

    function goToPreviousRoomFiltered(condition) {
        let index = listView.currentIndex;
        while (index-- !== 0) {
            if (condition(listView.itemAtIndex(index))) {
                listView.currentIndex = index;
                listView.currentItem.action.trigger();
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
        desiredWidth: page.width - Kirigami.Units.largeSpacing
        collapsed: page.collapsed
    }

    ListView {
        id: listView

        activeFocusOnTab: true
        clip: Accounts.count > 1

        header: QQC2.ItemDelegate {
            visible: page.collapsed
            action: Kirigami.Action {
                id: enterRoomAction
                onTriggered: quickView.item.open();
            }
            topPadding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            width: visible ? ListView.view.width : 0
            height: visible ? Kirigami.Units.gridUnit * 2 : 0

            Kirigami.Icon {
                anchors.centerIn: parent
                width: 22
                height: 22
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
            model: roomListModel
            onCurrentChanged: {
                listView.currentIndex = sortFilterRoomListModel.mapFromSource(current).row
            }
        }

        model: SortFilterRoomListModel {
            id: sortFilterRoomListModel
            sourceModel: RoomListModel {
                id: roomListModel
                connection: Controller.activeConnection
            }
            roomSortOrder: Config.mergeRoomList ? SortFilterRoomListModel.LastActivity : SortFilterRoomListModel.Categories
            onLayoutChanged: {
                listView.currentIndex = sortFilterRoomListModel.mapFromSource(itemSelection.currentIndex).row
            }
        }

        section.property: sortFilterRoomListModel.filterText.length === 0 && !Config.mergeRoomList ? "category" : null
        section.delegate: page.collapsed ? foldButton : sectionHeader

        Component {
            id: sectionHeader
            Kirigami.ListSectionHeader {
                height: implicitHeight
                label: roomListModel.categoryName(section)
                action: Kirigami.Action {
                    onTriggered: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
                }
                contentItem.children: QQC2.ToolButton {
                    icon.name: (roomListModel.categoryVisible(section) ? "go-up" : "go-down")
                    icon.width: Kirigami.Units.iconSizes.small
                    icon.height: Kirigami.Units.iconSizes.small

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

                    icon.name: hovered ? (roomListModel.categoryVisible(section) ? "go-up" : "go-down") : roomListModel.categoryIconName(section)
                    icon.width: Kirigami.Units.iconSizes.smallMedium
                    icon.height: Kirigami.Units.iconSizes.smallMedium

                    onClicked: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))

                    QQC2.ToolTip.text: roomListModel.categoryName(section)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }

        reuseItems: true
        currentIndex: -1 // we don't want any room highlighted by default

        delegate: page.collapsed ? collapsedModeListComponent : normalModeListComponent

        Component {
            id: collapsedModeListComponent

            QQC2.ItemDelegate {
                action: Kirigami.Action {
                    id: enterRoomAction
                    onTriggered: {
                        RoomManager.enterRoom(currentRoom);
                    }
                }
                Keys.onEnterPressed: enterRoomAction.trigger()
                Keys.onReturnPressed: enterRoomAction.trigger()
                topPadding: Kirigami.Units.largeSpacing
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                width: ListView.view.width
                height: visible ? ListView.view.width : 0
                visible: model.categoryVisible || sortFilterRoomListModel.filterText.length > 0 || Config.mergeRoomList

                contentItem: Kirigami.Avatar {
                    source: avatar ? "image://mxc/" + avatar : ""
                    name: model.name || i18n("No Name")
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }

                QQC2.ToolTip {
                    enabled: text.length !== 0
                    text: name ?? ""
                }
            }
        }

        Component {
            id: roomListContextMenu
            RoomListContextMenu {}
        }

        Component {
            id: normalModeListComponent
            Kirigami.BasicListItem {
                id: roomListItem
                visible: model.categoryVisible || sortFilterRoomListModel.filterText.length > 0 || Config.mergeRoomList
                topPadding: Kirigami.Units.largeSpacing
                bottomPadding: Kirigami.Units.largeSpacing
                highlighted: listView.currentIndex === index
                focus: true
                icon: undefined
                action: Kirigami.Action {
                    id: enterRoomAction
                    onTriggered: {
                        RoomManager.enterRoom(currentRoom);
                    }
                }
                Keys.onEnterPressed: enterRoomAction.trigger()
                Keys.onReturnPressed: enterRoomAction.trigger()
                bold: unreadCount > 0
                label: name ?? ""
                labelItem.textFormat: Text.PlainText
                subtitle: subtitleText
                subtitleItem.textFormat: Text.PlainText
                subtitleItem.visible: !Config.compactRoomList
                onPressAndHold: {
                    createRoomListContextMenu()
                }
                TapHandler {
                    acceptedButtons: Qt.RightButton
                    acceptedDevices: PointerDevice.Mouse
                    onTapped: createRoomListContextMenu()
                }

                leading: Kirigami.Avatar {
                    source: avatar ? "image://mxc/" + avatar : ""
                    name: model.name || i18n("No Name")
                    implicitWidth: visible ? height : 0
                    visible: Config.showAvatarInRoomDrawer
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }

                trailing: RowLayout {
                    Kirigami.Icon {
                        source: "notifications-disabled"
                        enabled: false
                        implicitWidth: Kirigami.Units.iconSizes.smallMedium
                        implicitHeight: Kirigami.Units.iconSizes.smallMedium
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        visible: currentRoom.pushNotificationState === PushNotificationState.Mute && !configButton.visible && unreadCount <= 0
                        Accessible.name: i18n("Muted room")
                    }
                    QQC2.Label {
                        id: notificationCountLabel
                        text: notificationCount > 0 ? notificationCount : "●"
                        visible: unreadCount > 0
                        color: Kirigami.Theme.textColor
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        Layout.minimumHeight: Kirigami.Units.iconSizes.smallMedium
                        Layout.minimumWidth: Math.max(notificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
                        horizontalAlignment: Text.AlignHCenter
                        background: Rectangle {
                            visible: notificationCount > 0
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                            color: highlightCount > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                            opacity: highlightCount > 0 ? 1 : 0.3
                            radius: height / 2
                        }

                        TextMetrics {
                            id: notificationCountTextMetrics
                            text: notificationCountLabel.text
                        }
                    }
                    QQC2.Button {
                        id: configButton
                        visible: roomListItem.hovered && !Kirigami.Settings.isMobile && !Config.compactRoomList
                        Accessible.name: i18n("Configure room")

                        action: Kirigami.Action {
                            id: optionAction
                            icon.name: "configure"
                            onTriggered: {
                                createRoomListContextMenu()
                            }
                        }
                    }
                }

                function createRoomListContextMenu() {
                    const menu = roomListContextMenu.createObject(page, {room: currentRoom})
                    if (!Kirigami.Settings.isMobile && !Config.compactRoomList) {
                        configButton.visible = true
                        configButton.down = true
                    }
                    menu.closed.connect(function() {
                        configButton.down = undefined
                        configButton.visible = Qt.binding(function() { return roomListItem.hovered && !Kirigami.Settings.isMobile && !Config.compactRoomList })
                    })
                    menu.open()
                }

                readonly property bool hasUnread: unreadCount > 0
            }
        }
    }

    footer: UserInfo {
        width: parent.width
        visible: !page.collapsed
    }

    MouseArea {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        parent: applicationWindow().overlay.parent

        x: page.currentWidth - width / 2
        width: Kirigami.Units.smallSpacing * 2
        z: page.z + 1
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
                    _private.currentWidth = Qt.binding(() => _private.collapsedSize + (page.contentItem.QQC2.ScrollBar.vertical.visible ? page.contentItem.QQC2.ScrollBar.vertical.width : 0));
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
