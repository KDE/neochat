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
    id: page

    title: i18n("Rooms")

    property var enteredRoom
    property bool collapsedMode: Config.roomListPageWidth === applicationWindow().collapsedPageWidth && applicationWindow().shouldUseSidebars

    onCollapsedModeChanged: if (collapsedMode) {
        sortFilterRoomListModel.filterText = "";
        if (page.contentItem && page.contentItem.flickableItem && page.contentItem.flickableItem.QQC2.ScrollBar.vertical) {
            page.contentItem.flickableItem.QQC2.ScrollBar.vertical.visible = false;
        }
    } else {
        page.contentItem.flickableItem.QQC2.ScrollBar.vertical.visible = true;
    }

    // HACK: the scrollbar is created with a 0 timer, so we need to set the visible flag
    // after it has been created
    Timer {
        running: true
        interval: 200 
        onTriggered: page.contentItem.flickableItem.QQC2.ScrollBar.vertical.visible = !collapsedMode;
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


    ListView {
        id: listView

        activeFocusOnTab: true
        clip: accountList.count > 1

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
                subtitle: {
                    let txt = (lastEvent.length === 0 ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm, " ")
                    if (txt.length) {
                        return txt
                    }
                    return " "
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
                        text: notificationCount
                        visible: notificationCount > 0
                        padding: Kirigami.Units.smallSpacing
                        color: highlightCount > 0 ? "white" : Kirigami.Theme.textColor
                        Layout.minimumWidth: height
                        horizontalAlignment: Text.AlignHCenter
                        background: Rectangle {
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                            color: highlightCount > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
                            radius: height / 2
                        }
                    }
                    QQC2.Button {
                        id: configButton
                        visible: roomListItem.hovered || Kirigami.Settings.isMobile
                        Accessible.name: i18n("Configure room")

                        action: Kirigami.Action {
                            id: optionAction
                            icon.name: "configure"
                            onTriggered: {
                                const menu = roomListContextMenu.createObject(page, {"room": currentRoom})
                                configButton.visible = true
                                configButton.down = true
                                menu.closed.connect(function() {
                                    configButton.down = undefined
                                    configButton.visible = Qt.binding(function() { return roomListItem.hovered || Kirigami.Settings.isMobile })
                                })
                                menu.popup()
                            }
                        }
                    }
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
                }
            }
        }
    }
}
