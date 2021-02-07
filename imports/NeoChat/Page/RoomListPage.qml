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
            sourceModel: RoomListModel {
                id: roomListModel
                connection: page.activeConnection
            }
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

        delegate: Kirigami.BasicListItem {
            id: roomListItem
            visible: model.categoryVisible || sortFilterRoomListModel.filterText.length > 0 || Config.mergeRoomList
            focus: true
            icon: undefined
            action: Kirigami.Action {
                id: enterRoomAction
                onTriggered: {
                    var roomItem = roomManager.enterRoom(currentRoom)
                    roomListItem.KeyNavigation.right = roomItem
                    roomItem.focus = true;
                }
            }

            label: name ?? ""
            subtitle: {
                let txt = (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm," ")
                if (txt.length) {
                    return txt
                }
                return " "
            }

            leading: Kirigami.Avatar {
                source: avatar ? "image://mxc/" + avatar : ""
                name: model.name || i18n("No Name")
                implicitWidth: height
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
