/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

import NeoChat.Component 1.0
import NeoChat.Menu 1.0

Kirigami.ScrollablePage {
    id: page

    property var roomListModel
    property var enteredRoom
    required property var activeConnection

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

    title: i18n("Rooms")

    titleDelegate: Kirigami.SearchField {
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true
        onTextChanged: sortFilterRoomListModel.filterText = text
        KeyNavigation.tab: listView
    }

    ListView {
        id: listView
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count == 0
            text: sortFilterRoomListModel.filterText.length > 0 ? i18n("No rooms found") : i18n("Join some rooms to get started")
            helpfulAction: Kirigami.Action {
                icon.name: sortFilterRoomListModel.filterText.length > 0 ? "search" : "list-add"
                text: sortFilterRoomListModel.filterText.length > 0 ? i18n("Search in room directory") : i18n("Explore rooms")
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {"connection": activeConnection, "keyword": sortFilterRoomListModel.filterText})
            }
        }
        model:  SortFilterRoomListModel {
            id: sortFilterRoomListModel
            sourceModel: roomListModel
            roomSortOrder: Config.mergeRoomList ? SortFilterRoomListModel.LastActivity : SortFilterRoomListModel.Categories
        }

        section.property: sortFilterRoomListModel.filterText.length === 0 && !Config.mergeRoomList ? "category" : null
        section.delegate: Kirigami.ListSectionHeader {
            id: sectionHeader
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
                }
                Kirigami.Icon {
                    source: roomListModel.categoryVisible(section) ? "go-up" : "go-down"
                    implicitHeight: Kirigami.Units.iconSizes.small
                    implicitWidth: Kirigami.Units.iconSizes.small
                }
            }
        }

        delegate: Kirigami.AbstractListItem {
            id: roomListItem
            property bool itemVisible: model.categoryVisible || sortFilterRoomListModel.filterText.length > 0 || Config.mergeRoomList
            visible: itemVisible
            highlighted: roomManager.currentRoom && roomManager.currentRoom.displayName === displayName
            focus: true
            action: Kirigami.Action {
                id: enterRoomAction
                onTriggered: {
                    if (category === RoomType.Invited) {
                        roomManager.openInvitation(currentRoom);
                    } else {
                        var roomItem = roomManager.enterRoom(currentRoom)
                        roomListItem.KeyNavigation.right = roomItem
                        roomItem.focus = true;
                    }
                }
            }


            contentItem: RowLayout {
                id: roomLayout
                spacing: Kirigami.Units.largeSpacing
                width: listView.width

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    onTapped: roomListContextMenu.createObject(roomLayout, {"room": currentRoom}).popup()
                }

                TapHandler {
                    onTapped: enterRoomAction.trigger()
                    onLongPressed: roomListContextMenu.createObject(roomLayout, {"room": currentRoom}).popup()
                }

                Kirigami.Avatar {
                    id: roomAvatar
                    property int size: Kirigami.Units.gridUnit * 2 + Kirigami.Units.smallSpacing
                    Layout.minimumHeight: size
                    Layout.maximumHeight: size
                    Layout.minimumWidth: size
                    Layout.maximumWidth: size

                    source: avatar ? ("image://mxc/" + avatar) : ""
                    name: model.name || i18n("No Name")
                }

                ColumnLayout {
                    id: roomitemcolumn
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: Kirigami.Units.gridUnit * 2
                    Layout.maximumHeight: Kirigami.Units.gridUnit * 2
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignHCenter

                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: displayName ?? ""
                        elide: Text.ElideRight
                        font.bold: unreadCount >= 0 || highlightCount > 0 || notificationCount > 0
                        wrapMode: Text.NoWrap
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        text: (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm," ")
                        visible: text.length > 0
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                        color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.7)
                    }
                }
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
            }
        }
        Component {
            id: roomListContextMenu
            RoomListContextMenu {}
        }
    }

    footer: RowLayout {
        visible: accountListTab.count > 1
        height: visible ? accountListTab.implicitHeight : 0
        Repeater {
            id: accountListTab
            model: AccountListModel { }
            delegate: QQC2.TabButton {
                checkable: true
                checked: Controller.activeConnection.user.id === model.connection.user.id
                onClicked: Controller.activeConnection = model.connection
                Layout.fillWidth: true
                text: model.user.id
            }
        }
    }
}
