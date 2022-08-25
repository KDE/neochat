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
import NeoChat.Component 1.0
import NeoChat.Menu 1.0

Kirigami.ScrollablePage {

    header: ColumnLayout {
        visible: !page.collapsedMode
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0

        ListView {
            id: spaceList
            property string activeSpaceId: ''

            orientation: Qt.Horizontal
            spacing: Kirigami.Units.largeSpacing
            clip:true
            visible: spaceList.count > 0

            Layout.preferredHeight: Kirigami.Units.gridUnit * 3
            Layout.fillWidth: true

            model: SortFilterSpaceListModel {
                id: sortFilterSpaceListModel
                sourceModel: RoomListModel {
                    id: spaceListModel
                    connection: Controller.activeConnection
                }
            }


            Connections {
                target: SpaceHierarchyCache
                function onSpaceHierarchyChanged() {
                    if (spaceList.activeSpaceId !== '') {
                        sortFilterRoomListModel.activeSpaceRooms = SpaceHierarchyCache.getRoomListForSpace(spaceList.activeSpaceId, false);
                    }
                }
            }

            header: QQC2.Control {
                contentItem: QQC2.RoundButton {
                    id: homeButton
                    flat: true
                    padding: Kirigami.Units.gridUnit / 2
                    icon.name: "home"
                    text: i18nc('@action:button', 'Show All Rooms')
                    display: QQC2.AbstractButton.IconOnly

                    onClicked: {
                        sortFilterRoomListModel.activeSpaceRooms = [];
                        spaceList.activeSpaceId = '';
                        listView.positionViewAtIndex(0, ListView.Beginning);
                    }

                    QQC2.ToolTip {
                        text: homeButton.text
                    }
                }
            }

            delegate: QQC2.Control {
                required property string avatar
                required property var currentRoom
                required property int index
                required property string id
                implicitWidth: ListView.view.headerItem.implicitWidth
                implicitHeight: ListView.view.headerItem.implicitHeight

                contentItem: Kirigami.Avatar {
                    id: del

                    actions.main: Kirigami.Action {
                        id: enterSpaceAction
                        onTriggered: {
                            spaceList.activeSpaceId = id;
                            sortFilterRoomListModel.activeSpaceRooms = SpaceHierarchyCache.getRoomListForSpace(id, true);
                        }
                    }

                    QQC2.ToolTip {
                        text: currentRoom.displayName
                    }

                    source: avatar !== "" ? "image://mxc/" + avatar : ""
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }
    id: page

    title: i18n("Rooms")

    property var enteredRoom
    property bool collapsedMode: Config.roomListPageWidth === applicationWindow().collapsedPageWidth && applicationWindow().shouldUseSidebars

    verticalScrollBarPolicy: collapsedMode ? QQC2.ScrollBar.AlwaysOff : QQC2.ScrollBar.AsNeeded

    onCollapsedModeChanged: if (collapsedMode) {
        sortFilterRoomListModel.filterText = "";
    }

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            itemSelection.setCurrentIndex(roomListModel.index(roomListModel.indexForRoom(RoomManager.currentRoom), 0), ItemSelectionModel.SelectCurrent)
        }
    }

    function goToNextRoom() {
        do {
            listView.incrementCurrentIndex();
        } while (!listView.currentItem.visible && listView.currentIndex === listView.count)
        listView.currentItem.action.trigger();
    }

    function goToPreviousRoom() {
        do {
            listView.decrementCurrentIndex();
        } while (!listView.currentItem.visible && listView.currentIndex !== 0)
        listView.currentItem.action.trigger();
    }

    titleDelegate: collapsedMode ? empty : searchField

    Component {
        id: empty
        Item {}
    }

    Component {
        id: searchField
        Kirigami.SearchField {
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.fillHeight: true
            Layout.fillWidth: true
            onTextChanged: sortFilterRoomListModel.filterText = text
            KeyNavigation.tab: listView
        }
    }

    ListView {
        id: listView

        activeFocusOnTab: true
        clip: accountList.count > 1

        header: QQC2.ItemDelegate {
            visible: page.collapsedMode
            action: Kirigami.Action {
                id: enterRoomAction
                onTriggered: quickView.item.open();
            }
            topPadding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            width: visible ? page.width : 0
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
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {
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
        section.delegate: Kirigami.ListSectionHeader {
            id: sectionHeader
            height: implicitHeight
            action: Kirigami.Action {
                onTriggered: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
            }
            contentItem: RowLayout {
                implicitHeight: categoryName.implicitHeight
                Kirigami.Heading {
                    id: categoryName
                    level: 3
                    text: roomListModel.categoryName(section)
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    visible: !page.collapsedMode
                }
                Kirigami.Icon {
                    source: page.collapsedMode ? roomListModel.categoryIconName(section) : (roomListModel.categoryVisible(section) ? "go-up" : "go-down")
                    implicitHeight: Kirigami.Units.iconSizes.small
                    implicitWidth: Kirigami.Units.iconSizes.small
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        reuseItems: true
        currentIndex: -1 // we don't want any room highlighted by default

        delegate: page.collapsedMode ? collapsedModeListComponent : normalModeListComponent

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
                height: ListView.view.width

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
                    visible: Config.showAvatarInTimeline
                    sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }

                trailing: RowLayout {
                    QQC2.Label {
                        text: notificationCount > 0 ? notificationCount : ""
                        visible: unreadCount > 0
                        padding: Kirigami.Units.smallSpacing
                        color: Kirigami.Theme.textColor
                        Layout.minimumWidth: height
                        horizontalAlignment: Text.AlignHCenter
                        background: Rectangle {
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                            color: highlightCount > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                            radius: height / 2
                        }
                    }
                    QQC2.Button {
                        id: configButton
                        visible: roomListItem.hovered
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
                    const menu = roomListContextMenu.createObject(page, {"room": currentRoom})
                    configButton.visible = true
                    configButton.down = true
                    menu.closed.connect(function() {
                        configButton.down = undefined
                        configButton.visible = Qt.binding(function() { return roomListItem.hovered || Kirigami.Settings.isMobile })
                    })
                    menu.open()
                }
            }
        }
    }

    footer: QQC2.ToolBar {
        visible: accountList.count > 1 && !collapsedMode
        height: visible ? implicitHeight : 0
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        contentItem: RowLayout {
            spacing: 0
            Repeater {
                id: accountList
                model: AccountRegistry
                delegate: Kirigami.BasicListItem {
                    checkable: true
                    checked: Controller.activeConnection && Controller.activeConnection.localUserId === model.connection.localUserId
                    onClicked: Controller.activeConnection = model.connection
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: model.connection.localUserId
                    subtitle: model.connection.localUser.accountLabel
                }
            }
        }
    }
}
